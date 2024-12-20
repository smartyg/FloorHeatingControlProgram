#include "MatchKey.h"

#include "Types.h"

bool QueryParser::MatchKey (const char* query, const QueryParser::Types::key_value_t* key_value, const char* key) noexcept {
	const QueryParser::Types::len_t len = key_value->key.len;
	const char* ptr = &(query[key_value->key.start]);

	std::size_t i = 0;
	while (true) {
		if (i >= len && key[i] == '\0') return true;
		else if (i >= len || key[i] == '\0') return false;
		else if (ptr[i] != key[i]) return false;
		++i;
	}
}
