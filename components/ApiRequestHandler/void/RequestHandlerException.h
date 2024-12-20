#ifndef REQUESTHANDLEREXCEPTION_H
#define REQUESTHANDLEREXCEPTION_H

#include <initializer_list>

#include <esp_http_server.h>
#include <cJSON.h>

#include "ApiRequestHandler.h"
#include "cJSON_AddToObject.h"

namespace RequestHandler::Exception {
class RequestHandlerException {
public:
    enum rh_except_t: std::uint8_t {
        UNKNOWN = 0,
        NO_ARGUMENTS_PROVIDED,
        INVALID_ARGUMENTS_PROVIDED,
        INVALID_REQUEST,
        TOO_LONG,
        KEY_NOY_PROVIDED
    };

private:
    const rh_except_t _type;
    char* _message;

    static constexpr const char* _pre_msg[] = {
        "An unknown error occured",
        "No arguments have been provided",
        "Invalid arguments have been provided",
        "HTTP Request is invalid",
        "URL is too long",
        "Required arguments has not been provided"
    };

public:
    RequestHandlerException (const rh_except_t& type) noexcept : _type(type) {
        this->_message = const_cast<char*>(this->_pre_msg[static_cast<uint8_t>(type)]);
    }

    inline constexpr rh_except_t getType (void) const noexcept { return this->_type; }
    inline constexpr const char* what (void) const noexcept { return this->_message; }

    esp_err_t return_error (httpd_req_t* request, const std::initializer_list<const char*> attr = {}) const noexcept {
        char* response;
        cJSON* json;
        try {
            json = cJSON_CreateObject ();
            cJSON_AddToObject<bool> (json, "error", true);
            cJSON_AddToObject<const_char_ptr> (json, "uri", request->uri);
            cJSON_AddToObject<const_char_ptr> (json, "message", this->_message);
            response = cJSON_Print (json);
        } catch (const std::exception& e) {
            return exception_during_api_call (request, e);
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
/*
private:
    inline static constexpr const char* message (const rh_except_t& type) {
        switch (type) {
            case RequestHandlerException::NO_ARGUMENTS_PROVIDED:
                const char* msg = "No arguments have been provided";
                return msg;
                break;
            case RequestHandlerException::INVALID_ARGUMENTS_PROVIDED:
                const char* msg = "No arguments have been provided";
                return msg;
                break;
            case RequestHandlerException::INVALID_REQUEST:
                const char* msg = "HTTP Request is invalid";
                return msg;
                break;
            case RequestHandlerException::TOO_LONG:
                const char* msg = "URL is too long";
                return msg;
                break;
            case RequestHandlerException::KEY_NOY_PROVIDED:
                const char* msg = "Required arguments has not been provided";
                return msg;
                break;
            case RequestHandlerException::UNKNOWN:
            default:
                const char* msg = "An unknown error occured";
                return msg;
                break;
        }
    }*/
};


class RequestHandlerExceptionNoKey : public RequestHandlerException {
public:
    RequestHandlerExceptionNoKey (const char* key) : RequestHandlerException (KEY_NOY_PROVIDED)
};
}

#endif /* REQUESTHANDLEREXCEPTION_H */
