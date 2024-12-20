#ifndef HTTPSERVER_HANDLERENTRY_H
#define HTTPSERVER_HANDLERENTRY_H

#include <esp_http_server.h>

#include "Types.h"
#include "HandlerData.h"

namespace HttpServer {
	class HandlerEntry {
	private:
		const char* _uri;    /*!< The URI to handle */
		httpd_method_t _method; /*!< Method supported by the URI, HTTP_ANY for wildcard method to support all methods*/
		esp_err_t (*_worker_task_handle)(httpd_req_t *r);
		void* _user_ctx;
		bool _is_websocket;
		bool _handle_ws_control_frames;
		const char* _supported_subprotocol;
		const http_server_handler_t _real_handler;
		void* _user_data;

	public:
		constexpr HandlerEntry (const void* const null = nullptr) : _uri(nullptr), _method(static_cast<httpd_method_t>(HTTP_ANY)), _user_ctx(nullptr), _is_websocket(false), _real_handler(nullptr), _user_data(nullptr) {}

		constexpr HandlerEntry (const char* uri, httpd_method_t method, http_server_handler_t const& handler, void* user_data) : _uri(uri), _method(method), _is_websocket(false), _handle_ws_control_frames(nullptr), _supported_subprotocol(nullptr), _real_handler(handler), _user_data(user_data) {
			this->_user_ctx = reinterpret_cast<HandlerData*>(const_cast<http_server_handler_t*>(&(this->_real_handler)));
		}

		constexpr HandlerEntry (const char* uri, httpd_method_t method, bool handle_ws_control_frames, const char* supported_subprotocol, http_server_handler_t const& handler, void* user_data) : _uri(uri), _method(method), _is_websocket(true), _handle_ws_control_frames(handle_ws_control_frames), _supported_subprotocol(supported_subprotocol), _real_handler(handler), _user_data(user_data) {
			this->_user_ctx = reinterpret_cast<HandlerData*>(const_cast<http_server_handler_t*>(&(this->_real_handler)));
		}

		inline const httpd_uri_t* getHttpdUri (esp_err_t (*worker_handler)(httpd_req_t *r)) {
			this->_worker_task_handle = worker_handler;
			return reinterpret_cast<const httpd_uri_t*>(&(this->_uri));
		}

		inline constexpr bool isNull (void) const {
			return (this->_uri == nullptr && this->_real_handler == nullptr);
		}

		inline constexpr const char* getUri (void) const {
			return this->_uri;
		}

		inline constexpr httpd_method_t getMethod (void) const {
			return this->_method;
		}
	};
}

#endif /* HTTPSERVERHANDLERENTRY_H */
