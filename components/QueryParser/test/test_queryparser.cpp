#include "unity.h"
#include "QueryParser.h"
#include "Types.h"
#include "MatchKey.h"

// Mock definitions and includes
#include <esp_heap_caps.h>

// Mock function to replace heap_caps_malloc and heap_caps_free
void* mock_heap_caps_malloc(size_t size, uint32_t caps) {
	return malloc(size);
}

void mock_heap_caps_free(void* ptr) {
	free(ptr);
}

static QueryParser::Parser* parser = nullptr;

void setUp(void) {
	// Initialize the parser object
	const char* query = "key1=value1&key2=value2";
	parser = new QueryParser::Parser(query, false);
}

void tearDown(void) {
	// Clean up the parser object
	delete parser;
	parser = nullptr;
}

void test_QueryParserStart(void) {
	TEST_ASSERT_NOT_NULL(parser);
}

void test_QueryParserHasKey(void) {
	QueryParser::Types::record_t record = parser->hasKey("key1");
	TEST_ASSERT_NOT_EQUAL(-1, record);

	record = parser->hasKey("key3");
	TEST_ASSERT_EQUAL(-1, record);
}

void test_QueryParserGetValue(void) {
	const char* value = parser->getValue("key1");
	TEST_ASSERT_EQUAL_STRING("value1", value);

	value = parser->getValue("key2");
	TEST_ASSERT_EQUAL_STRING("value2", value);

	value = parser->getValue("key3");
	TEST_ASSERT_NULL(value);
}

void test_QueryParserCopyValue(void) {
	char buffer[10];
	std::size_t len = parser->copyValue("key1", buffer, sizeof(buffer));
	TEST_ASSERT_EQUAL_STRING("value1", buffer);
	TEST_ASSERT_EQUAL(7, len); // includes null terminator

	len = parser->copyValue("key2", buffer, sizeof(buffer));
	TEST_ASSERT_EQUAL_STRING("value2", buffer);
	TEST_ASSERT_EQUAL(7, len); // includes null terminator

	len = parser->copyValue("key3", buffer, sizeof(buffer));
	TEST_ASSERT_EQUAL(0, len);
}
/*
extern "C" void app_main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_QueryParserStart);
	RUN_TEST(test_QueryParserHasKey);
	RUN_TEST(test_QueryParserGetValue);
	RUN_TEST(test_QueryParserCopyValue);
	UNITY_END();
}
*/
