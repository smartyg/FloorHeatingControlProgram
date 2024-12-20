set(COMPONENT_ADD_INCLUDEDIRS "include")
set(COMPONENT_SRCS "Exception.cpp" "ApiRequestHandler.cpp")
set(COMPONENT_REQUIRES "esp_http_server" "QueryParser" "json" "cJSON_AddToObject")

register_component()
