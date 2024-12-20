#ifndef HTTPSERVER_TYPES_H
#define HTTPSERVER_TYPES_H

#include <functional>

#include "esp_http_server.h"

namespace HttpServer {
    typedef std::function<esp_err_t(httpd_req_t*, void*)> http_server_handler_t;
}

#endif /* HTTPSERVER_TYPES_H */
