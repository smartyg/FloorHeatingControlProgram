#ifndef ASYNCREQUEST_H
#define ASYNCREQUEST_H

#include "Types.h"

namespace HttpServer {
	struct AsyncRequest {
		httpd_req_t* request;
		const http_server_handler_t handler;
		void* user_data;
	};
}

#endif /* ASYNCREQUEST_H */
