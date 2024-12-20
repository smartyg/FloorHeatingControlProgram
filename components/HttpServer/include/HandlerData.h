#ifndef HTTPSERVER_HANDLERDATA_H
#define HTTPSERVER_HANDLERDATA_H

#include "Types.h"

namespace HttpServer {
	struct HandlerData {
		const http_server_handler_t handler;
		void* user_data;
	};
}

#endif /* HTTPSERVER_HANDLERDATA_H */
