#include "utils.h"

#include <cstdint>

#include <esp_heap_caps.h>

char* HassMqtt::utils::strdup (const char* const str) {
	if(str == nullptr) return nullptr;

	int len = 0;
	while (str[len] != '\0') { ++len; }
	char* new_str = static_cast<char*>(heap_caps_malloc ((len + 1) * sizeof (char), MALLOC_CAP_8BIT));
	for (int i = 0; i <= len; ++i) {
		new_str[i] = str[i];
	}
	return new_str;
}

void HassMqtt::utils::genId (const uint32_t& id, char* buf) {
    for (uint8_t i = 0; i < 8; ++i) {
        uint8_t v = (id >> (i * 4)) & 15;
        buf[i] = static_cast<char>(v + (v < 10 ? 48 : (97 - 10)));
    }
}
