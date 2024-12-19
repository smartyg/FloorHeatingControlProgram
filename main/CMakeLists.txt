idf_component_register(SRCS "DebugTaskInfo.cpp" "wifi_connect.c" "TaskControl.cpp" "loop.cpp" "main.cpp"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES "ds18b20" "esp_http_server" "nvs_flash" "json" "MqttCpp" "QueryParser" "HttpServer" "ApiRequestHandler" "HassMqtt")
