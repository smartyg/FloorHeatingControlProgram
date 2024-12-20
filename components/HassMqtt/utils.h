#ifndef HASSMQTT_UTILS_H
#define HASSMQTT_UTILS_H

#include <cstdint>
#include <cstdlib>

#include <esp_heap_caps.h>

namespace HassMqtt::utils {
	inline int countLen (int& len, const int& delimiter_len, const char* str) {
		len += delimiter_len;
		for (int i = 0; str[i] != '\0'; ++i) ++len;
		return len;
	}

	template<typename T, typename... ArgTypes>
	inline int countLen (int& len, const int& delimiter_len, const T str, ArgTypes... args) {
		len += delimiter_len;
		for (int i = 0; str[i] != '\0'; ++i) ++len;
		return countLen (len, delimiter_len, args...);
	}

	inline void append (char* ptr, const char& delimiter, const char* str) {
		if (delimiter != '\0') *(ptr++) = delimiter;
		for (int i = 0; str[i] != '\0'; ++i) *(ptr++) = str[i];
	}

	template<typename T, typename... ArgTypes>
	inline void append (char* ptr, const char& delimiter, const T str, ArgTypes... args) {
		if (delimiter != '\0') *(ptr++) = delimiter;
		for (int i = 0; str[i] != '\0'; ++i) *(ptr++) = str[i];
		append (ptr, delimiter, args...);
	}

	template<typename... ArgTypes>
	char* makeTopic (const char& delimiter, const char* str, ArgTypes... args) {
		int len = 0;
		for (int i = 0; str[i] != '\0'; ++i) ++len;
		len = countLen (len, (delimiter == '\0' ? 0 : 1), args...);

		char* buf = static_cast<char*>(heap_caps_malloc ((len + 1) * sizeof (char), MALLOC_CAP_8BIT));

		char* ptr = buf;
		for (int i = 0; str[i] != '\0'; ++i) *(ptr++) = str[i];
		append (ptr, delimiter, args...);
		buf[len] = '\0';

		return buf;
	}

	char* strdup (const char* const str);
	void genId (const uint32_t& id, char* buf);
}

#endif /* HASSMQTT_UTILS_H */
