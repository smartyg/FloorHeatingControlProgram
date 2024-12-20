#ifndef APIREQUESTHANDLER_H
#define APIREQUESTHANDLER_H

#include <cstdint>
#include <functional>

#include <esp_http_server.h>

#define API_getter_0(entry, mode, inst, func, type, txt, data) { entry, mode, [&inst] (httpd_req_t* request, void* ptr)->esp_err_t { return ApiRequestHandler::get_type<type> (static_cast<std::function<type(void)>>(std::bind (func, inst)), txt, request, ptr); }, data }
#define API_getter_1(entry, mode, inst, func, type, txt, data) { entry, mode, [&inst] (httpd_req_t* request, void* ptr)->esp_err_t { return ApiRequestHandler::get_type<type> (static_cast<std::function<type(const uint8_t&)>>(std::bind (func, inst, std::placeholders::_1)), txt, request, ptr); }, data }

#define API_setter_0(entry, mode, inst, func, type, txt, data) { entry, mode, [&inst] (httpd_req_t* request, void* ptr)->esp_err_t { return ApiRequestHandler::set_type<type> (static_cast<std::function<bool(const type&)>>(std::bind (func, inst, std::placeholders::_1)), txt, request, ptr); }, data }
#define API_setter_1(entry, mode, inst, func, type, txt, data) { entry, mode, [&inst] (httpd_req_t* request, void* ptr)->esp_err_t { return ApiRequestHandler::set_type<type> (static_cast<std::function<bool(const uint8_t&, const type&)>>(std::bind (func, inst, std::placeholders::_1, std::placeholders::_2)), txt, request, ptr); }, data }

namespace ApiRequestHandler {
	template<typename T>
	concept IsBaseType = std::same_as<T, bool> || std::is_integral_v<T> || std::same_as<T, float> || std::same_as<T, char*> || std::same_as<T, const char*>;

	template<typename T>
	concept IsEnumType = std::is_enum_v<T> && IsBaseType<std::underlying_type_t<T>>;

	template<typename T> requires IsBaseType<T>
	esp_err_t get_type (std::function<T(void)> const& getter_func, const char* return_attribute, httpd_req_t* request, void* data);

	template<typename T, typename U = std::underlying_type_t<T>> requires IsEnumType<T>
	inline esp_err_t get_type (std::function<T(void)> const& getter_func, const char* return_attribute, httpd_req_t* request, void* data) {
		std::function<U(void)> const getter_wrapper_func = [&getter_func] (void) -> U { return static_cast<const U>(getter_func ()); };
		return get_type<U> (getter_wrapper_func, return_attribute, request, data);
	}

	template<typename T> requires IsBaseType<T>
	esp_err_t get_type (std::function<T(const uint8_t&)> const& getter_func, const char* return_attribute, httpd_req_t* request, void* data);

	template<typename T, typename U = std::underlying_type_t<T>> requires IsEnumType<T>
	inline esp_err_t get_type (std::function<T(const uint8_t&)> const& getter_func, const char* return_attribute, httpd_req_t* request, void* data) {
		std::function<U(const uint8_t&)> const getter_wrapper_func = [&getter_func] (const uint8_t& i) -> U { return static_cast<const U>(getter_func (i)); };
		return get_type<U> (getter_wrapper_func, return_attribute, request, data);
	}

	template<typename T> requires IsBaseType<T>
	esp_err_t set_type (std::function<bool(const T&)> const& setter_func, const char* return_attribute, httpd_req_t* request, void* data);

	template<typename T, typename U = std::underlying_type_t<T>> requires IsEnumType<T>
	inline esp_err_t set_type (std::function<bool(const T&)> const& setter_func, const char* return_attribute, httpd_req_t* request, void* data) {
		std::function<bool(const U&)> const setter_wrapper_func = [&setter_func] (const U& v) -> bool { return setter_func (static_cast<const T>(v)); };
		return set_type<U> (setter_wrapper_func, return_attribute, request, data);
	}

	template<typename T> requires IsBaseType<T>
	esp_err_t set_type (std::function<bool(const uint8_t&, const T&)> const& setter_func, const char* return_attribute, httpd_req_t* request, void* data);

	template<typename T, typename U = std::underlying_type_t<T>> requires IsEnumType<T>
	inline esp_err_t set_type (std::function<bool(const uint8_t&, const T&)> const& setter_func, const char* return_attribute, httpd_req_t* request, void* data) {
		std::function<bool(const uint8_t&, const U&)> const setter_wrapper_func = [&setter_func] (const uint8_t& i, const U& v) -> bool { return setter_func (i, static_cast<const T>(v)); };
		return set_type<U> (setter_wrapper_func, return_attribute, request, data);
	}
}

#endif /* APIREQUESTHANDLER_H */
