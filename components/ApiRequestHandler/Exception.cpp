#include "Exception.h"

#include <initializer_list>
#include <stdexcept>
#include <esp_check.h>
#include <esp_http_server.h>
#include <cJSON.h>
#include <cJSON_AddToObject.h>

esp_err_t ApiRequestHandler::Exception::exception_during_api_call (httpd_req_t* request, const std::exception& e) {
	httpd_resp_set_status (request, "500 Internal Server Error");
	httpd_resp_set_type (request, "application/json");
	cJSON* json = cJSON_CreateObject ();
	cJSON_AddToObject<bool> (json, "error", true);
	cJSON_AddToObject<const_char_ptr> (json, "uri", request->uri);
	cJSON_AddToObject<const_char_ptr> (json, "message", e.what ());
	const char *response = cJSON_Print (json);
	httpd_resp_send (request, response, HTTPD_RESP_USE_STRLEN);
	cJSON_Delete (json);
	return ESP_OK;
}

esp_err_t ApiRequestHandler::Exception::Exception::return_error (httpd_req_t* request, const std::initializer_list<const char*> attr) const noexcept {
	char* response;
	cJSON* json;
	try {
		json = cJSON_CreateObject ();
		cJSON_AddToObject<bool> (json, "error", true);
		cJSON_AddToObject<const_char_ptr> (json, "uri", request->uri);
		cJSON_AddToObject<const_char_ptr> (json, "message", this->_message);
		response = cJSON_Print (json);
	} catch (const std::exception& e) {
		return ApiRequestHandler::Exception::exception_during_api_call (request, e);
	}

	if (this->_type == UNKNOWN)
		httpd_resp_set_status (request, "500 Internal Server Error");
	else
		httpd_resp_set_status (request, "400 Bad Request");

	httpd_resp_set_type (request, "application/json");
	httpd_resp_send (request, response, HTTPD_RESP_USE_STRLEN);

	cJSON_Delete (json);
	return ESP_OK;
}
