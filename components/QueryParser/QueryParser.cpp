#include "Types.h"
#include "QueryParser.h"

#include <esp_heap_caps.h>

#include "MatchKey.h"

QueryParser::Parser::Parser (const char* query, const bool& free_query) : _query(query), _free_query(free_query) {
	QueryParser::Types::start_t current_key_start = 0;
	QueryParser::Types::start_t current_value_start = 0;
	QueryParser::Types::len_t key_len = 0;
	QueryParser::Types::len_t value_len = 0;
	bool look_for_key = true;

	for (start_t i = 0; true; ++i) {
		if (this->_query[i] == '&' || this->_query[i] == '\0') { // end of key or key/value pair found, safe it
			if (look_for_key) key_len = i - current_key_start;
			else value_len = i - current_value_start;
			QueryParser::Types::key_t key = { current_key_start, key_len };
			QueryParser::Types::value_t value = { current_value_start, value_len };
			this->addKeyValue (key, value);

			if (this->_query[i] == '\0') break;

			current_key_start = i + 1;
			current_value_start = 0;
			key_len = 0;
			value_len = 0;
			look_for_key = true;
		} else if (look_for_key && this->_query[i] == '=') { // end of key found, now look for value
			look_for_key = false;
			current_value_start = i + 1;
			key_len = i - current_key_start;
		}
	}
}

QueryParser::Parser::~Parser (void) {
	heap_caps_free (this->_key_values);
	if (this->_free_query) heap_caps_free (const_cast<char*>(this->_query));
	this->_key_values = nullptr;
	this->_key_value_size = 0;
	this->_key_value_capacity = 0;
}

QueryParser::Types::record_t QueryParser::Parser::hasKey (const char* key) const {
	for (QueryParser::Types::record_t i = 0; i < this->_key_value_size; ++i) {
		if (QueryParser::MatchKey (this->_query, &(this->_key_values[i]), key)) return i;
	}
	return -1;
}

std::size_t QueryParser::Parser::getValueLen (const QueryParser::Types::record_t& r) const noexcept {
	if (r < 0 || r >= this->_key_value_size) return 0;
	return this->_key_values[r].value.len;
}

const char* QueryParser::Parser::getValue (const QueryParser::Types::record_t& r) const noexcept {
	if (r < 0 || r >= this->_key_value_size) return nullptr;
	if (this->_key_values[r].value.start == 0) return nullptr;
	return &(this->_query[this->_key_values[r].value.start]);
}

const char* QueryParser::Parser::getValue (const char* key) const noexcept {
	const QueryParser::Types::record_t r = this->hasKey (key);
	return this->getValue (r);
}

std::size_t QueryParser::Parser::copyValue (const QueryParser::Types::record_t& r, char* buf, const std::size_t& buf_len) const noexcept {
	if (r < 0 || r >= this->_key_value_size) return 0;
	if (this->_key_values[r].value.start == 0) {
		if (buf_len > 0) buf [0] = '\0';
		return 0;
	}
	len_t value_len = this->_key_values[r].value.len;
	const char* ptr = &(this->_query[this->_key_values[r].value.start]);
	std::size_t i = 0;
	for (; i < buf_len && i < value_len; ++i) {
		buf[i] = ptr[i];
	}
	if (i < buf_len) buf[i++] = '\0';
	return i;
}

std::size_t QueryParser::Parser::copyValue (const char* key, char* buf, const std::size_t& buf_len) const noexcept {
	const record_t r = this->hasKey (key);
	return this->copyValue (r, buf, buf_len);
}

void QueryParser::Parser::addKeyValue (const QueryParser::Types::key_t& key, const QueryParser::Types::value_t& value) {
	if (this->_key_value_size >= this->_key_value_capacity) {
		this->_key_value_capacity += 10;
		this->_key_values = static_cast<key_value_t*>(heap_caps_realloc (static_cast<void*>(this->_key_values), this->_key_value_capacity * sizeof (key_value_t), MALLOC_CAP_8BIT));
	}
	this->_key_values[this->_key_value_size] = { key, value };
	++this->_key_value_size;
}
/*
static bool QueryParser::Parser::matchKey (const char* query, const QueryParser::Types::key_value_t* key_value, const char* key) noexcept {
	const len_t len = key_value->key.len;
	const char* ptr = &(query[key_value->key.start]);

	std::size_t i = 0;
	while (true) {
		if (i >= len && key[i] == '\0') return true;
		else if (i >= len || key[i] == '\0') return false;
		else if (ptr[i] != key[i]) return false;
		++i;
	}
}*/
