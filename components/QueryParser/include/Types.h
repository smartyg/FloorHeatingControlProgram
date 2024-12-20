#ifndef QUERYPARSER_TYPES_H
#define QUERYPARSER_TYPES_H

#include <cstdint>

namespace QueryParser {
	inline namespace Types {
		typedef int16_t record_t;
		typedef uint16_t start_t;
		typedef uint16_t len_t;

		struct StartLen {
			start_t start;
			len_t len;
		};

		typedef StartLen key_t;
		typedef StartLen value_t;

		struct KeyValue {
			key_t key;
			value_t value;
		};
		typedef KeyValue key_value_t;
	}
}

#endif /* QUERYPARSER_TYPES_H */
