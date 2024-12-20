#include "ApiRequestHandler.h"

#include <cstdint>
#include <stdexcept>
#include <cmath>
#include <type_traits>
#include <functional>
#include <source_location>

#include <cJSON.h>
//#include <esp_log.h>
#include <esp_http_server.h>

#include <QueryParser.h>
#include <cJSON_AddToObject.h>

#include "Exception.h"

static const char* TAG = "RequestHandler";

#ifndef CHAR_PTR
#define CHAR_PTR
typedef char* char_ptr;
typedef const char* const_char_ptr;
#endif /* CHAR_PTR */

typedef const char* query_str_t;

esp_err_t request_return_json (httpd_req_t* request, const cJSON* const json) noexcept {
	//ESP_LOGV (TAG, "%s::%s:%ld (%p, %p)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(), request, json);
	httpd_resp_set_status (request, "200 OK");
	httpd_resp_set_type (request, "application/json");
	const char *response = cJSON_Print (json);
	httpd_resp_send (request, response, HTTPD_RESP_USE_STRLEN);
	return ESP_OK;
}

template<typename T>
esp_err_t return_single_value (httpd_req_t* request, const char* return_attribute, const T& value) {
	//ESP_LOGV (TAG, "%s::%s:%ld (%p, %p, ...)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(), request, return_attribute);
	cJSON* json = cJSON_CreateObject ();
	cJSON_AddToObject<T> (json, return_attribute, value);
	esp_err_t ret = request_return_json (request, json);
	cJSON_Delete (json);
	return ret;
}
/*
esp_err_t exception_during_api_call (httpd_req_t* request, const std::exception& e) {
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
*/
template<typename T>
T convert_to_value (const char* buf, const std::size_t& len) = delete;
/*
template<typename T> requires std::is_same_v<T, Status::control_mode_t>
T convert_to_value (const char* buf, const std::size_t& len) {
	if (len == 1 && buf[0] == '1') return static_cast<Status::control_mode_t>(1);
	else if (len == 1 && buf[0] == '0') return static_cast<Status::control_mode_t>(0);
	throw std::runtime_error ("Provided value is not a reconized boolean value.");
}
*/
template<typename T> requires std::is_same_v<T, bool>
T convert_to_value (const char* buf, const std::size_t& len) {
	if (len == 1) {
		switch (buf[0]) {
			case '1':
			case 'y':
			case 'Y':
				return true;
			case '0':
			case 'n':
			case 'N':
				return false;
		}
	} else if (len == 2 && (buf[0] == 'N' || buf[0] == 'n') && (buf[1] == 'O' || buf[1] == 'o')) return false;
	else if (len == 3 && (buf[0] == 'Y' || buf[0] == 'y') && (buf[1] == 'E' || buf[1] == 'e') && (buf[2] == 'S' || buf[2] == 's')) return true;
	else if (len == 4 && (buf[0] == 'T' || buf[0] == 't') && (buf[1] == 'R' || buf[1] == 'r') && (buf[2] == 'U' || buf[2] == 'u') && (buf[3] == 'E' || buf[3] == 'e')) return true;
	else if (len == 5 && (buf[0] == 'F' || buf[0] == 'f') && (buf[1] == 'A' || buf[1] == 'a') && (buf[2] == 'L' || buf[2] == 'l') && (buf[3] == 'S' || buf[3] == 's') && (buf[4] == 'E' || buf[4] == 'e')) return false;
	throw std::runtime_error ("Provided value is not a reconized boolean value.");
}

template<typename T> requires ( std::is_arithmetic_v<T> && !std::is_same_v<T, bool> )
T convert_to_value (const char* buf, const std::size_t& len) {
	std::remove_cv_t<T> value = 0;
	std::size_t i = 0;
	bool is_negative = false;
	int8_t decimal = 0;

	//if T is signed type
	if (buf[0] == '-') {
		if constexpr (!std::is_signed_v<T>) throw std::runtime_error ("Type is not a signed type, but '-' sign was found.");;
		is_negative = true;
		++i;
	} else if (buf[0] == '+') {
		is_negative = false;
		++i;
	}
	for (; i < len; ++i) {
		switch (buf[i]) {
			case '0':
				if (decimal == 0) {
					value = (value * 10) + 0;
				} else {
					//value += 0 * (10^decimal);
					--decimal;
				}
				break;
			case '1':
				if (decimal == 0) {
					value = (value * 10) + 1;
				} else {
					value += 1 * std::pow (10, decimal);
					--decimal;
				}
				break;
			case '2':
				if (decimal == 0) {
					value = (value * 10) + 2;
				} else {
					value += 2 * std::pow (10, decimal);
					--decimal;
				}
				break;
			case '3':
				if (decimal == 0) {
					value = (value * 10) + 3;
				} else {
					value += 3 * std::pow (10, decimal);
					--decimal;
				}
				break;
			case '4':
				if (decimal == 0) {
					value = (value * 10) + 4;
				} else {
					value += 4 * std::pow (10, decimal);
					--decimal;
				}
				break;
			case '5':
				if (decimal == 0) {
					value = (value * 10) + 5;
				} else {
					value += 5 * std::pow (10, decimal);
					--decimal;
				}
				break;
			case '6':
				if (decimal == 0) {
					value = (value * 10) + 6;
				} else {
					value += 6 * std::pow (10, decimal);
					--decimal;
				}
				break;
			case '7':
				if (decimal == 0) {
					value = (value * 10) + 7;
				} else {
					value += 7 * std::pow (10, decimal);
					--decimal;
				}
				break;
			case '8':
				if (decimal == 0) {
					value = (value * 10) + 8;
				} else {
					value += 8 * std::pow (10, decimal);
					--decimal;
				}
				break;
			case '9':
				if (decimal == 0) {
					value = (value * 10) + 9;
				} else {
					value += 9 * std::pow (10, decimal);
					--decimal;
				}
				break;
			case '.':
				if constexpr (std::is_floating_point_v<T>) {
					if (!decimal)
						decimal = -1;
					else throw std::runtime_error ("Decimal dot was already found in number.");
				} else throw std::runtime_error ("Decimal dot was found while type is not a decimal number.");
			break;
			default:
				throw std::runtime_error ("Unreconized charachter found.");
		}
	}
	if (is_negative) value *= -1;
	return value;
}

template<typename T> requires std::is_same_v<std::remove_cv_t<T>, char*>
T convert_to_value (const char* buf, const std::size_t& len) {
	std::remove_cv_t<T> value = static_cast<std::remove_cv_t<T>>(heap_caps_malloc ((len + 1) * sizeof (char), MALLOC_CAP_8BIT));
	for (std::size_t i = 0; i < len; ++i) value[i] = buf[i];
	value[len] = '\0';
	return value;
}

template<typename T>
const T get_value_from_request_query (const QueryParser::Parser* qp, const char* key) {
	//ESP_LOGV (TAG, "%s::%s:%ld (%p, %s)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(), qp, key);
	QueryParser::record_t r = qp->hasKey (key);
	if (r == -1) throw ApiRequestHandler::Exception::ExceptionNoKey (key);

	return convert_to_value<T> (qp->getValue (r), qp->getValueLen (r));
}

const QueryParser::Parser* getQueryParser (httpd_req_t* request) {
	//ESP_LOGD (TAG, "%s (%p) :%ld", std::source_location::current ().function_name (), request, std::source_location::current ().line ());
	std::size_t query_len = httpd_req_get_url_query_len (request) + 1;
	if (query_len == 1) throw ApiRequestHandler::Exception::Exception (ApiRequestHandler::Exception::NO_ARGUMENTS_PROVIDED);

	char* query = static_cast<char*>(heap_caps_malloc (query_len * sizeof (char), MALLOC_CAP_8BIT));
	esp_err_t ret = httpd_req_get_url_query_str (request, query, query_len);
	//ESP_LOGD (TAG, "len: %d; pointer: %p; string: %s", query_len, query, query);
	if (ret == ESP_OK) {
		return new QueryParser::Parser (query, true);
	}
	else heap_caps_free (query);
	switch (ret) {
		case ESP_ERR_NOT_FOUND:
			throw ApiRequestHandler::Exception::Exception (ApiRequestHandler::Exception::NO_ARGUMENTS_PROVIDED);
		case ESP_ERR_INVALID_ARG:
			throw ApiRequestHandler::Exception::Exception (ApiRequestHandler::Exception::INVALID_ARGUMENTS_PROVIDED);
		case ESP_ERR_HTTPD_INVALID_REQ:
			throw ApiRequestHandler::Exception::Exception (ApiRequestHandler::Exception::INVALID_REQUEST);
		case ESP_ERR_HTTPD_RESULT_TRUNC:
			throw ApiRequestHandler::Exception::Exception (ApiRequestHandler::Exception::TOO_LONG);
		default:
			throw ApiRequestHandler::Exception::Exception (ApiRequestHandler::Exception::UNKNOWN);
	}
	return nullptr;
}

template<typename T> requires ApiRequestHandler::IsBaseType<T>
esp_err_t ApiRequestHandler::set_type (std::function<bool(const T&)> const& setter_func, const char* return_attribute, httpd_req_t* request, void* data) {
	//ESP_LOGD (TAG, "%s::%s:%ld (%p, %s, %p, %p)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(), setter_func.target<bool(*)(const T)> (), return_attribute, request, data);
	//ESP_LOGD (TAG, "%s::%s:%ld (..., %s, %p, %p)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(),  return_attribute, request, data);
	try {
		const QueryParser::Parser* qp = getQueryParser (request);
		const T value = get_value_from_request_query<T> (qp, "value");
		bool success = setter_func (value);
		return return_single_value (request, return_attribute, success);
	} catch (const std::exception& e) {
		return ApiRequestHandler::Exception::exception_during_api_call (request, e);
	} catch (const ApiRequestHandler::Exception::Exception& e) {
		return e.return_error (request, {"value"});
	}
}

template<typename T> requires ApiRequestHandler::IsBaseType<T>
esp_err_t ApiRequestHandler::set_type (std::function<bool(const uint8_t&, const T&)> const& setter_func, const char* return_attribute, httpd_req_t* request, void* data) {
	//ESP_LOGD (TAG, "%s::%s:%ld (%p, %s, %p, %p)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(), setter_func.target<bool(*)(const uint8_t, const T)> (), return_attribute, request, data);
	//ESP_LOGD (TAG, "%s::%s:%ld (..., %s, %p, %p)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(),  return_attribute, request, data);
	try {
		const QueryParser::Parser* qp = getQueryParser (request);
		const uint8_t idx = get_value_from_request_query<uint8_t> (qp, "id");
		const T value = get_value_from_request_query<T> (qp, "value");
		bool success = setter_func (idx, value);
		return return_single_value (request, return_attribute, success);
	} catch (const std::exception& e) {
		return ApiRequestHandler::Exception::exception_during_api_call (request, e);
	} catch (const ApiRequestHandler::Exception::Exception& e) {
		return e.return_error (request, {"id", "value"});
	}
}

template<typename T> requires ApiRequestHandler::IsBaseType<T>
esp_err_t ApiRequestHandler::get_type (std::function<T(void)> const& getter_func, const char* return_attribute, httpd_req_t* request, void* data) {
	//ESP_LOGD (TAG, "%s::%s:%ld (%p, %s, %p, %p)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(), getter_func.target<T(*)(void)> (), return_attribute, request, data);
	//ESP_LOGD (TAG, "%s::%s:%ld (..., %s, %p, %p)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(),  return_attribute, request, data);
	try {
		const T value = getter_func ();
		return return_single_value (request, return_attribute, value);
	} catch (const std::exception& e) {
		return ApiRequestHandler::Exception::exception_during_api_call (request, e);
	} catch (const ApiRequestHandler::Exception::Exception& e) {
		return e.return_error (request);
	}
}

template<typename T> requires ApiRequestHandler::IsBaseType<T>
esp_err_t ApiRequestHandler::get_type (std::function<T(const uint8_t&)> const& getter_func, const char* return_attribute, httpd_req_t* request, void* data) {
	//ESP_LOGD (TAG, "%s::%s:%ld (%p, %s, %p, %p)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(), getter_func.target<T(*)(const uint8_t)> (), return_attribute, request, data);
	//ESP_LOGD (TAG, "%s::%s:%ld (..., %s, %p, %p)", TAG, std::source_location::current ().function_name(), std::source_location::current ().line(),  return_attribute, request, data);
	try {
		const QueryParser::Parser* qp = getQueryParser (request);
		const uint8_t idx = get_value_from_request_query<uint8_t> (qp, "id");
		const T value = getter_func (idx);
		return return_single_value (request, return_attribute, value);
	} catch (const std::exception& e) {
		return ApiRequestHandler::Exception::exception_during_api_call (request, e);
	} catch (const ApiRequestHandler::Exception::Exception& e) {
		return e.return_error (request, {"id"});
	}
}

template esp_err_t ApiRequestHandler::set_type<bool> (std::function<bool(const bool&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<signed char> (std::function<bool(const signed char&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<unsigned char> (std::function<bool(const unsigned char&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<signed int> (std::function<bool(const signed int&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<unsigned int> (std::function<bool(const unsigned int&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<float> (std::function<bool(const float&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<char_ptr> (std::function<bool(const char_ptr&)> const&, const char*, httpd_req_t*, void*);

template esp_err_t ApiRequestHandler::set_type<bool> (std::function<bool(const uint8_t&, const bool&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<signed char> (std::function<bool(const uint8_t&, const signed char&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<unsigned char> (std::function<bool(const uint8_t&, const unsigned char&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<signed int> (std::function<bool(const uint8_t&, const signed int&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<unsigned int> (std::function<bool(const uint8_t&, const unsigned int&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<float> (std::function<bool(const uint8_t&, const float&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::set_type<char_ptr> (std::function<bool(const uint8_t&, const char_ptr&)> const&, const char*, httpd_req_t*, void*);

template esp_err_t ApiRequestHandler::get_type<bool> (std::function<bool(void)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<signed char> (std::function<signed char(void)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<unsigned char> (std::function<unsigned char(void)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<signed int> (std::function<signed int(void)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<unsigned int> (std::function<unsigned int(void)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<float> (std::function<float(void)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<const_char_ptr> (std::function<const_char_ptr(void)> const&, const char*, httpd_req_t*, void*);

template esp_err_t ApiRequestHandler::get_type<bool> (std::function<bool(const uint8_t&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<signed char> (std::function<signed char(const uint8_t&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<unsigned char> (std::function<unsigned char(const uint8_t&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<signed int> (std::function<signed int(const uint8_t&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<unsigned int> (std::function<unsigned int(const uint8_t&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<float> (std::function<float(const uint8_t&)> const&, const char*, httpd_req_t*, void*);
template esp_err_t ApiRequestHandler::get_type<const_char_ptr> (std::function<const_char_ptr(const uint8_t&)> const&, const char*, httpd_req_t*, void*);
