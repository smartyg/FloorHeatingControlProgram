#include "Server.h"

#include <functional>
#include <exception>
#include <algorithm>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_heap_caps.h>
#include <esp_http_server.h>
#include <esp_log.h>

#include "HandlerEntry.h"
#include "WorkerData.h"
#include "AsyncRequest.h"

static const char* TAG = "HttpServer";

HttpServer::Server::~Server (void) {
	// Force stop the server, and kill any pending action.

	// Remove the reference to ourselfs.
	this->_httpd_config.global_user_ctx = nullptr;

	// Indicate that we are not accepting any new requests anymore.
	this->_running = false;

	// Delete the queue so any waiting requests will be removed
	if (this->_async_req_queue != nullptr) {
		vQueueDelete (this->_async_req_queue);
		this->_async_req_queue = nullptr;
	}

	// Force kill all existing workers
	for (int i = 0; i < this->_num_workers; ++i) {
		if (this->_worker_task_handle[i] != nullptr) {
			vTaskDelete (this->_worker_task_handle[i]);
			this->_worker_task_handle[i] = nullptr;
		}
	}

	// Stop the httpd server
	if (this->_server != nullptr) {
		httpd_stop (this->_server);
		this->_server = nullptr;
	}

	// and free the reservation for the workers task handle
	if (this->_worker_task_handle != nullptr) {
		heap_caps_free (this->_worker_task_handle);
		this->_worker_task_handle = nullptr;
	}
}

bool HttpServer::Server::start (HttpServer::HandlerEntry* api) {
	if (this->_running != false || this->_server != nullptr || this->_async_req_queue != nullptr ) return false;

	this->_async_req_queue = xQueueCreate (this->_queue_size, sizeof (HttpServer::AsyncRequest));

	std::size_t num_api = 0;
	while (!api[num_api].isNull ()) {
				++num_api;
			}

	this->createWorkers ();
	this->_httpd_config.global_user_ctx = this;
	this->_httpd_config.global_user_ctx_free_fn = [](void* ctx) -> void { return; };
	this->_httpd_config.max_uri_handlers = num_api;

	esp_err_t ret;
	if ((ret = httpd_start (&this->_server, &this->_httpd_config)) == ESP_OK) {
		// Give the server time to start
		vTaskDelay (100);

		// Register URI handlers
		if (api == nullptr) {
			api = this->_stored_api;
		}
		if (api != nullptr) {
			for (int i = 0; !(api[i].isNull ()); ++i) {
				httpd_register_uri_handler (this->_server, api[i].getHttpdUri (&HttpServer::Server::asyncHandler));
			}
			this->_stored_api = api;
		}
		this->_running = true;
		return true;
	}
	ESP_LOGE (TAG, "Failed to start HTTP Server: %d", ret);
	return false;
}

bool HttpServer::Server::stop (void) {
	// Gracefully stop the server.

	try {
		// Indicate that we are not accepting any new requests anymore.
		this->_running = false;

		// Now that we are not accepting any new requests, wait till the message queue is empty before deleting it.
		if (this->_async_req_queue != nullptr) {
			while (uxQueueMessagesWaiting (this->_async_req_queue) > 0) {
				vTaskDelay (pdMS_TO_TICKS (10));
			}

			// Message queue is empty, now delete it.
			vQueueDelete (this->_async_req_queue);
			this->_async_req_queue = nullptr;
		}

		// Wait till all the works finish their task and delete themselfs.
		if (this->_worker_task_handle != nullptr) {
			for (int i = 0; i < this->_num_workers; ++i) {
				if (this->_worker_task_handle[i] != nullptr) {
					eTaskState state = eTaskGetState (this->_worker_task_handle[i]);
					while (state != eDeleted && state != eInvalid) {
						// Worker might still be working on something, let's suspend ourselfs to give time for finishing the worker.
						vTaskDelay (pdMS_TO_TICKS (100));
						state = eTaskGetState (this->_worker_task_handle[i]);
					}
					this->_worker_task_handle[i] = nullptr;
				}
			}
		}
		// Wait 10 ms to give time to other processes before we finally stop the server.
		vTaskDelay (pdMS_TO_TICKS (10));

		// Stop the httpd server
		if (this->_server != nullptr) {
			httpd_stop (this->_server);
			this->_server = nullptr;
		}
	} catch (const std::exception &e) {
		return false;
	}
	return true;
}

void HttpServer::Server::createWorkers (void) {
	void* worker_argument = reinterpret_cast<HttpServer::WorkerData*>(&(this->_async_req_queue));

	// start worker tasks
	for (int i = 0; i < this->_num_workers; ++i) {
		try {
			int ret = xTaskCreate (HttpServer::Server::worker,
								   "async request worker",
						  this->_worker_stack_size, // stack size
						  worker_argument,
						  this->_worker_task_priority, // priority
						  &(this->_worker_task_handle[i]));
			if (ret != pdPASS) {
				ESP_LOGE (TAG, "Failed to start asyncReqWorker: %d", ret);
				continue;
			}
		} catch (const std::exception& e) {
			ESP_LOGE (TAG, "exception creating worker: %s", e.what ());
		}
		vTaskDelay (100);
	}
}

esp_err_t HttpServer::Server::asyncHandler (httpd_req_t* req) {
	ESP_LOGD (TAG, "Receive request: %s", req->uri);

	// must create a copy of the request that we own
	httpd_req_t* copy = nullptr;
	esp_err_t err = httpd_req_async_handler_begin (req, &copy);
	if (err != ESP_OK) {
		return err;
	}

	const HttpServer::HandlerData* handler_data = static_cast<const HttpServer::HandlerData*>(copy->user_ctx);
	const HttpServer::Server* const server = static_cast<HttpServer::Server*>(httpd_get_global_user_ctx (copy->handle));
	if (server == nullptr) {
		ESP_LOGE (TAG, "Failed to get pointer to the main Http Server");
		return ESP_FAIL;
	}

	HttpServer::AsyncRequest async_request = {
		.request = copy,
		.handler = handler_data->handler,
		.user_data = handler_data->user_data,
	};

	try {
		ESP_LOGD (TAG, "forward request to worker: %s", async_request.request->uri);
		QueueHandle_t queue = server->getQueue ();
		if (server->isRunning () == false || queue == nullptr || xQueueSendToBack (queue, &async_request, pdMS_TO_TICKS (100)) == false) {
			ESP_LOGE (TAG, "worker queue is full");
			httpd_req_async_handler_complete (copy); // cleanup
			return ESP_FAIL;
		}
	} catch (const std::exception& e) {
		ESP_LOGE (TAG, "exception placing request in queue: %s", e.what ());
		return ESP_FAIL;
	}
	return ESP_OK;
}

void HttpServer::Server::worker (void* ptr) {
	ESP_LOGI (TAG, "Child has started.");
	const HttpServer::WorkerData* data = static_cast<const HttpServer::WorkerData*>(ptr);
	const TickType_t delay_tick = std::max (data->worker_wait_ticks / data->num_workers / 2, static_cast<TickType_t>(10));
	HttpServer::AsyncRequest async_request;

	while (data->queue != nullptr) {
		// Receive a message on the created queue.  Block for 10 ticks if a message is not immediately available.
		if(xQueueReceive (data->queue, &(async_request), data->worker_wait_ticks) == pdTRUE) {
			ESP_LOGD (TAG, "got request for: %s", async_request.request->uri);
			try {
				if (async_request.handler != nullptr) {
					async_request.handler (async_request.request, async_request.user_data);
				}
				if (httpd_req_async_handler_complete (async_request.request) != ESP_OK) {
					ESP_LOGE (TAG, "failed to complete async req");
				}
			} catch (const std::exception& e) {
				ESP_LOGE (TAG, "Can not call function handler: %s", e.what ());
			}
		} else {
			vTaskDelay (delay_tick);
		}
	}
	ESP_LOGI (TAG, "Message queue has been removed, stop worker now.");

	vTaskDelete (nullptr);
}

esp_err_t HttpServer::Server::showServerHandlers (httpd_req_t* request, void*) const {
	constexpr char html_start[] = "<!DOCTYPE html><html><head><title>Overview of api endpoints</title></head><body>";
	constexpr char html_end[] = "</body></html>";
	constexpr char html_api_part_1[] = "<p><a href=\"";
	constexpr char html_api_part_2[] = "\">";
	constexpr char html_api_part_3[] = "</a> (";
	constexpr char html_api_part_4[] = ")</p>";

	esp_err_t res;

	res = httpd_resp_send_chunk (request, html_start, sizeof (html_start) - 1);

	for (int i = 0; !(this->_stored_api[i].isNull ()); ++i) {
		size_t uri_len = 0;
		const char* uri = this->_stored_api[i].getUri ();
		while (uri[uri_len] != '\0') { ++uri_len; }

		res = httpd_resp_send_chunk (request, html_api_part_1, sizeof (html_api_part_1) - 1);
		res = httpd_resp_send_chunk (request, uri, uri_len);
		res = httpd_resp_send_chunk (request, html_api_part_2, sizeof (html_api_part_2) - 1);
		res = httpd_resp_send_chunk (request, uri, uri_len);
		res = httpd_resp_send_chunk (request, html_api_part_3, sizeof (html_api_part_3) - 1);

		switch (this->_stored_api[i].getMethod ()) {
			case HTTP_GET:
				res = httpd_resp_send_chunk (request, "GET", 3);
				break;
			case HTTP_POST:
				res = httpd_resp_send_chunk (request, "POST", 4);
				break;
			default:
				res = httpd_resp_send_chunk (request, "unknown", 7);
				break;
		}

		res = httpd_resp_send_chunk (request, html_api_part_4, sizeof (html_api_part_4) - 1);
	}
	res = httpd_resp_send_chunk (request, html_end, sizeof (html_end) - 1);
	res = httpd_resp_send_chunk (request, nullptr, 0);
	return ESP_OK;
}
