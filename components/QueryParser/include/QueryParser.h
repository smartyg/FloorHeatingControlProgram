#ifndef QUERYPARSER_PARSER_H
#define QUERYPARSER_PARSER_H

#include <cstdlib>
#include <cstdint>

#include "Types.h"

namespace QueryParser {
	class Parser {
	private:
		const char* _query;
		key_value_t* _key_values = nullptr;
		record_t _key_value_size = 0;
		record_t _key_value_capacity = 0;
		const bool _free_query = false;

	public:
		//Parser (const char* query, const bool& free_query) : _query(query), _free_query(free_query);
		Parser (const char* query, const bool& free_query);// : _query(query), _free_query(free_query);
		~Parser (void);
		record_t hasKey (const char* key) const;
		std::size_t getValueLen (const record_t& r) const noexcept;
		const char* getValue (const record_t& r) const noexcept;
		const char* getValue (const char* key) const noexcept;
		std::size_t copyValue (const record_t& r, char* buf, const std::size_t& buf_len) const noexcept;
		std::size_t copyValue (const char* key, char* buf, const std::size_t& buf_len) const noexcept;

	private:
		void addKeyValue (const key_t& key, const value_t& value);
		static bool matchKey (const char* query, const key_value_t* key_value, const char* key) noexcept;
	};
}

#endif /* QUERYPARSER_PARSER_H */
