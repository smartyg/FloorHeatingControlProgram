#ifndef CJSON_ADDTOOBJECT_H
#define CJSON_ADDTOOBJECT_H

#include <cJSON.h>

#ifndef CHAR_PTR
#define CHAR_PTR
typedef char* char_ptr;
typedef const char* const_char_ptr;
#endif /* CHAR_PTR */
/*
template<typename T>
cJSON* cJSON_AddToObject (cJSON* const object, const char* const name, const T& value) = delete;

 * template<typename T> requires (std::is_arithmetic_v<T> && !std::same_as<T, bool>)
 * cJSON* cJSON_AddToObject (cJSON* const object, const char* const name, const T& value) {
 *	return cJSON_AddNumberToObject (object, name, value);
 * }
 *
 * template <>
 * inline cJSON* cJSON_AddToObject<bool> (cJSON* const object, const char* const name, const bool& value) {
 *	return cJSON_AddBoolToObject (object, name, value);
 * }
 *
 * template <>
 * inline cJSON* cJSON_AddToObject<char_ptr> (cJSON* const object, const char* const name, const char_ptr& value) {
 *	return cJSON_AddStringToObject (object, name, value);
 * }
 *
 * template <>
 * inline cJSON* cJSON_AddToObject<const_char_ptr> (cJSON* const object, const char* const name, const const_char_ptr& value) {
 *	return cJSON_AddStringToObject (object, name, value);
 * }
 */

template<typename T>
inline cJSON* cJSON_Create (const T& value) = delete;

template<typename T> requires (std::is_arithmetic_v<T> && !std::same_as<T, bool>)
inline cJSON* cJSON_Create (const T& value) {
	return cJSON_CreateNumber (value);
}

template <>
inline cJSON* cJSON_Create<bool> (const bool& value) {
	return cJSON_CreateBool (value);
}

template <>
inline cJSON* cJSON_Create<char_ptr> (const char_ptr& value) {
	return cJSON_CreateString (value);
}

template <>
inline cJSON* cJSON_Create<const_char_ptr> (const const_char_ptr& value) {
	return cJSON_CreateString (value);
}


template<typename T>
inline cJSON* cJSON_AddToObject (cJSON* const object, const char* const name, const T& value) {
	cJSON* item = cJSON_Create<T> (value);
	cJSON_AddItemToObject (object, name, item);
	return object;
}


template<typename T>
bool cJSON_GetValue (const cJSON* const json, T& value) = delete;

template<typename T> requires (std::is_arithmetic_v<T> && !std::same_as<T, bool>)
inline bool cJSON_GetValue (const cJSON* const json, T& value) {
	if (cJSON_IsNumber (json)) {
		double value2 = cJSON_GetNumberValue (json);
		value = static_cast<T>(value2);
		return true;
	}
	return false;
}

template <>
inline bool cJSON_GetValue (const cJSON* const json, bool& value) {
	if (cJSON_IsBool (json)) {
		value = cJSON_IsTrue (json);
		return true;
	}
	return false;
}

template <>
inline bool cJSON_GetValue (const cJSON* const json, char*& value) {
	if (cJSON_IsString (json)) {
		value = cJSON_GetStringValue (json);
		return true;
	}
	return false;
}

#endif /* CJSON_ADDTOOBJECT_H */
