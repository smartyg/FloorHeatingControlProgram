#ifndef QUERYPARSER_MATCHKEY_H
#define QUERYPARSER_MATCHKEY_H

#include "MatchKey.h"

#include "Types.h"

namespace QueryParser {
	bool MatchKey (const char* query, const QueryParser::Types::key_value_t* key_value, const char* key) noexcept;
}

#endif /* QUERYPARSER_MATCHKEY_H */
