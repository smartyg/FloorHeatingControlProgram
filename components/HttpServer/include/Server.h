#ifndef HTTPSERVER_SERVER_H
#define HTTPSERVER_SERVER_H

#include <stdexcept>
#include <exception>
#include <algorithm>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_http_server.h>

#include "Types.h"
#include "HandlerEntry.h"

namespace HttpServer {
	class Server {
	private:
		httpd_handle_t _server = nullptr;
		httpd_config_t _httpd_config;
		size_t _queue_size = 8;
		QueueHandle_t _async_req_queue = nullptr;
		const size_t _num_workers = 2;
		const TickType_t _worker_wait_ticks = pdMS_TO_TICKS (1000);
		bool _running = false;
		TaskHandle_t* _worker_task_handle;
		const uint32_t _worker_stack_size = 4096;
		const uint8_t _worker_task_priority = 5;
		HandlerEntry* _stored_api = nullptr;

	public:
		Server () : Server (8, 2) {}

		Server (const size_t queue_size, const size_t num_workers) : _num_workers(num_workers) {
			this->_queue_size = std::max (queue_size, num_workers);
			this->_worker_task_handle = static_cast<TaskHandle_t*>(heap_caps_malloc (this->_num_workers * sizeof (TaskHandle_t), MALLOC_CAP_32BIT | MALLOC_CAP_DEFAULT));
			this->_httpd_config = HTTPD_DEFAULT_CONFIG ();
			this->_httpd_config.stack_size = 4096;
			this->_httpd_config.task_priority = 6;
			this->_httpd_config.core_id = 1;
		}

		~Server (void);

		bool start (HandlerEntry* api = nullptr);
		bool stop (void);

		inline bool isRunning (void) const { return (this->_running && this->_server != nullptr); }
		inline void setPort (uint16_t port) { if (this->isRunning ()) throw std::runtime_error ("function can only be called before the HTTP Server starts."); this->_httpd_config.server_port = port; }
		inline void setControlPort (uint16_t port) { if (this->isRunning ()) throw std::runtime_error ("function can only be called before the HTTP Server starts."); this->_httpd_config.ctrl_port = port; }
		inline void setCore (BaseType_t core) { if (this->isRunning ()) throw std::runtime_error ("function can only be called before the HTTP Server starts."); this->_httpd_config.core_id = core; }
		inline void setTaskPriority (unsigned task_priority) { if (this->isRunning ()) throw std::runtime_error ("function can only be called before the HTTP Server starts."); this->_httpd_config.task_priority = task_priority; }
		inline void setStackSize (size_t stack_size) { if (this->isRunning ()) throw std::runtime_error ("function can only be called before the HTTP Server starts."); this->_httpd_config.stack_size = stack_size; }
		inline void setMaxOpenSockets (uint16_t max_open_sockets) { if (this->isRunning ()) throw std::runtime_error ("function can only be called before the HTTP Server starts."); this->_httpd_config.max_open_sockets = max_open_sockets; }

		esp_err_t showServerHandlers (httpd_req_t* request, void*) const;

	private:
		void createWorkers (void);

		inline QueueHandle_t getQueue (void) const { return this->_async_req_queue; }

		static esp_err_t asyncHandler (httpd_req_t* req);
		static void worker (void* ptr);
	};
}

#endif /* HTTPSERVER_SERVER_H */
