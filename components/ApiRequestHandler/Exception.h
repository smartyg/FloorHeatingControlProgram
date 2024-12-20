#ifndef APIREQUESTHANDLER_EXCEPTION_H
#define APIREQUESTHANDLER_EXCEPTION_H

#include <cstdint>
#include <initializer_list>
#include <exception>
#include <esp_check.h>
#include <esp_http_server.h>

namespace ApiRequestHandler::Exception {
    esp_err_t exception_during_api_call (httpd_req_t* request, const std::exception& e);

    enum rh_except_t: std::uint8_t {
        UNKNOWN = 0,
        NO_ARGUMENTS_PROVIDED,
        INVALID_ARGUMENTS_PROVIDED,
        INVALID_REQUEST,
        TOO_LONG,
        KEY_NOY_PROVIDED
    };

    class Exception {
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
        Exception (const rh_except_t& type) noexcept : _type(type) {
            this->_message = const_cast<char*>(this->_pre_msg[static_cast<uint8_t>(type)]);
        }

        inline constexpr rh_except_t getType (void) const noexcept { return this->_type; }
        inline constexpr const char* what (void) const noexcept { return this->_message; }

        esp_err_t return_error (httpd_req_t* request, const std::initializer_list<const char*> attr = {}) const noexcept;
    };

    class ExceptionNoKey : public Exception {
    public:
        ExceptionNoKey (const char* key) : Exception (KEY_NOY_PROVIDED) {}
    };
}

#endif /* APIREQUESTHANDLER_EXCEPTION_H */
