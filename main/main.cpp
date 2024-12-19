#include <cstdio>

//#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

#include <HttpServer.h>
#include <ApiRequestHandler.h>
#include <MqttCpp.h>
#include <HassMqtt.h>
#include <cJSON_AddToObject.h>

#include "Status.h"
#include "wifi_connect.h"
#include "TaskControl.h"
#include "DebugTaskInfo.h"

// Define the cores
static constexpr uint8_t CORE_0 = 0;
static constexpr uint8_t CORE_1 = 1;

TaskHandle_t task_handle_control;
TaskHandle_t task_handle_debug;

static constexpr const char *ssid = "homenetwork-1420";          // Change this to your WiFi SSID
static constexpr const char *password = "s_T1a5GqnSbC$_0C";  // Change this to your WiFi password

static constexpr const char *TAG = "main";

static void disconnect_handler (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	HttpServer::Server* server = static_cast<HttpServer::Server*>(arg);
	if (server != nullptr) {
		ESP_LOGI (TAG, "Stopping webserver\n");
		if (!server->stop ()) {
			ESP_LOGW (TAG, "Failed to stop http server\n");
		}
	}
}

static void connect_handler (void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	HttpServer::Server* server = static_cast<HttpServer::Server*>(arg);
	if (server != nullptr) {
		ESP_LOGI (TAG, "Starting webserver\n");
		server->start ();
	}
}

template<typename T>
const HassMqtt::Attribute::Getter getFactory (const std::function<T(void)> callback) {
	const HassMqtt::Attribute::Getter lambda = [&callback] (const char* const, const char* const, void*) -> cJSON* {
		if (callback != nullptr)
			return cJSON_Create<T>(callback ());
		ESP_LOGW (TAG, "Callback points to null\n");
		return cJSON_CreateNull ();
	};
	return lambda;
}

template<typename T>
const HassMqtt::Attribute::Setter setFactory (const std::function<bool(const T)> callback) {
	const HassMqtt::Attribute::Setter lambda = [&callback] (const cJSON* const json, const char* const, const char* const, void*) -> void {
		T value;
		if (cJSON_GetValue<T>(json, value)) {
			if (callback != nullptr) {
				if (callback (value))
					return;
				else
					ESP_LOGW (TAG, "Callback function returned false.\n");
			}
			ESP_LOGW (TAG, "Callback points to null\n");
		}
	};
	return lambda;
}

template <>
inline bool cJSON_GetValue (const cJSON* const json, Status::ControlMode& value) {
	if (cJSON_IsBool (json)) {
		value = (cJSON_IsTrue (json) ? Status::ControlMode::AUTOMATIC : Status::ControlMode::MANUAL);
		return true;
	}
	return false;
}

void setup (void) {
	vTaskDelay (10);

	esp_log_level_set("*", ESP_LOG_INFO);
	esp_log_level_set(TAG, ESP_LOG_VERBOSE);
	esp_log_level_set("MqttCpp", ESP_LOG_VERBOSE);
	esp_log_level_set("HassMqtt", ESP_LOG_VERBOSE);
	esp_log_level_set("HttpServer", ESP_LOG_VERBOSE);

	ESP_LOGI (TAG, "Welcome at Floor Heating Control Program!\n");

	ESP_LOGD (TAG, "Free heap: %ld bytes\n", esp_get_free_heap_size ());

	ESP_LOGI (TAG, "Start WiFi...\n");

	vTaskDelay (pdMS_TO_TICKS (10));

	try {
		wifi_init_sta (WIFI_AUTH_WPA_WPA2_PSK, ssid, password);
	} catch (const std::exception &e) {
		ESP_LOGE (TAG, "exception found: %s\n", e.what ());
	}

	Status* status = new Status ();
	ESP_LOGD (TAG, "Status object created with address: %p\n", status);

	MqttCpp::Client* mqtt = new MqttCpp::Client ();
	ESP_LOGD (TAG, "Mqtt client object created with address: %p\n", mqtt);

	HttpServer::Server* server = new HttpServer::Server (16, 4);
	ESP_LOGD (TAG, "HttpServer object created with address: %p\n", server);

	static HttpServer::HandlerEntry api_functions[] = {
		API_getter_0 ("/mode/is_auto", HTTP_GET, status, &Status::is_mode_auto, bool, "auto", nullptr),
		API_getter_0 ("/mode/is_manual", HTTP_GET, status, &Status::is_mode_manual, bool, "manual", nullptr),

		API_setter_0 ("/mode/set", HTTP_GET, status, &Status::set_mode, Status::ControlMode, "sucess", nullptr),

		API_getter_0 ("/target_temperature/get", HTTP_GET, status, &Status::get_target_temperature, float, "target_temperature", nullptr),
		API_setter_0 ("/target_temperature/set", HTTP_GET, status, &Status::set_target_temperature, float, "sucess", nullptr),

		API_getter_0 ("/target_temperature_range/get", HTTP_GET, status, &Status::get_target_temperature_range, float, "target_temperature_range", nullptr),
		API_setter_0 ("/target_temperature_range/set", HTTP_GET, status, &Status::set_target_temperature_range, float, "sucess", nullptr),

		API_getter_0 ("/inlet_open/get", HTTP_GET, status, &Status::get_inlet_open, bool, "inlet_open", nullptr),
		API_setter_0 ("/inlet_open/set", HTTP_GET, status, &Status::set_inlet_open, bool, "sucess", nullptr),

		API_getter_0 ("/inlet_open/is_open", HTTP_GET, status, &Status::get_is_inlet_open, bool, "is_inlet_open", nullptr),

		API_getter_1 ("/zone_open/get", HTTP_GET, status, &Status::get_zone_open, bool, "zone_open", nullptr),
		API_setter_1 ("/zone_open/set", HTTP_GET, status, &Status::set_zone_open, bool, "sucess", nullptr),

		API_getter_1 ("/zone_open/is_open", HTTP_GET, status, &Status::get_is_zone_open, bool, "is_zone_open", nullptr),

		API_getter_1 ("/temperature/get", HTTP_GET, status, &Status::get_temperature, float, "temperature", nullptr),

		API_getter_1 ("/message_template/get", HTTP_GET, status, &Status::get_msg, const char*, "message_template", nullptr),
		//API_setter_1 ("/message_template/set", HTTP_POST, status, &Status::set_msg, char*, "sucess", nullptr),

		API_getter_1 ("/mqtt/get", HTTP_GET, status, &Status::get_zone_open, bool, "zone_open", nullptr),
		API_setter_1 ("/mqtt/set", HTTP_GET, status, &Status::set_zone_open, bool, "sucess", nullptr),
		//{ "/mqtt/get", HTTP_GET, [&mqtt] (httpd_req_t* request, void* user_data) { return server->showServerHandlers (request, user_data); }, nullptr },
		//{ "/mqtt/set", HTTP_GET, [&mqtt] (httpd_req_t* request, void* user_data) { return server->showServerHandlers (request, user_data); }, nullptr },

		//std::function<esp_err_t(httpd_req_t*, void*)>
		//("/mqtt/set", HTTP_GET, http_server_handler_t const& handler, nullptr)

		//{ "/status", HTTP_GET, &show_server_status, nullptr },
		//{ "/", HTTP_GET, static_cast<std::function<esp_err_t(httpd_req_t*, void*)>>(std::bind (&HttpServer::Server::showServerHandlers, server, std::placeholders::_1, std::placeholders::_2)), nullptr },
		{ "/", HTTP_GET, [&server] (httpd_req_t* request, void* user_data) { return server->showServerHandlers (request, user_data); }, nullptr },

		{ nullptr }
	};

	try {
		/* Start the server for the first time */
		ESP_LOGI (TAG, "Start HTTP server...\n");
		server->setPort (80);
		if (!server->start (api_functions))
			ESP_LOGE (TAG, "Error starting HTTP server");
	} catch (const std::exception &e) {
		ESP_LOGE (TAG, "exception found: %s\n", e.what ());
	}

	ESP_ERROR_CHECK (esp_event_handler_register (IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
	ESP_ERROR_CHECK (esp_event_handler_register (WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

	vTaskDelay (pdMS_TO_TICKS (100));

	ESP_LOGI (TAG, "Start MQTT...\n");

	HassMqtt::Discovery* hass;

	try {
		mqtt->setUri ("mqtt://10.0.1.15:1883");
		mqtt->connect ();

		while (!mqtt->isConnected ()) {
			ESP_LOGD (TAG, "mqtt is not connected yet, wait and try again.");
			vTaskDelay (pdMS_TO_TICKS (500));
		}

		hass = new HassMqtt::Discovery (mqtt, "FHCP2mqtt", "Martijn Goedhart", "esp32-s3", "FHCP2mqtt", "0.1");
		ESP_LOGD (TAG, "HassMqtt object created with address: %p\n", hass);
		auto ep_inlet = hass->endpoint ("FHCP2mqtt/inlet", true);
		ep_inlet->attribute<HassMqtt::Attribute::Types::SWITCH> ("state", getFactory<bool> (std::bind (&Status::get_inlet_open, status)), setFactory<bool> (std::bind (&Status::set_inlet_open, status, std::placeholders::_1)), "None", nullptr);
		ep_inlet->attribute<HassMqtt::Attribute::Types::SELECT> ("mode", getFactory<bool> (std::bind (&Status::is_mode_auto, status)), setFactory<Status::ControlMode> (std::bind (&Status::set_mode, status, std::placeholders::_1)), std::vector<const char*>({"manual", "automatic"}), nullptr);
		ep_inlet->attribute<HassMqtt::Attribute::Types::BINARY_SENSOR> ("is_open", getFactory<bool> (std::bind (&Status::get_is_inlet_open, status)), "opening", nullptr);
		ep_inlet->attribute<HassMqtt::Attribute::Types::SENSOR> ("temperature", getFactory<float> (std::bind (&Status::get_temperature, status, 0)), "temperature", "measurement", "°C", nullptr);
		ep_inlet->attribute<HassMqtt::Attribute::Types::NUMBER> ("target_temperature", getFactory<float> (std::bind (&Status::get_target_temperature, status)), setFactory<float> (std::bind (&Status::set_target_temperature, status, std::placeholders::_1)), "°C", 5, 50, 0.1, nullptr);
		ep_inlet->attribute<HassMqtt::Attribute::Types::NUMBER> ("target_temperature_range", getFactory<float> (std::bind (&Status::get_target_temperature_range, status)), setFactory<float> (std::bind (&Status::set_target_temperature_range, status, std::placeholders::_1)), "", 0, 10, 0.1, nullptr);

		auto ep_zone_1 = hass->endpoint ("FHCP2mqtt/zone/1");
		ep_zone_1->attribute<HassMqtt::Attribute::Types::SWITCH> ("state", getFactory<bool> (std::bind (&Status::get_zone_open, status, 1)), setFactory<bool> (std::bind (&Status::set_zone_open, status, 1, std::placeholders::_1)), "None", nullptr);
		ep_zone_1->attribute<HassMqtt::Attribute::Types::BINARY_SENSOR> ("is_open", getFactory<float> (std::bind (&Status::get_is_zone_open, status, 1)), "opening", nullptr);

		auto ep_zone_2 = hass->endpoint ("FHCP2mqtt/zone/2");
		ep_zone_2->attribute<HassMqtt::Attribute::Types::SWITCH> ("state", getFactory<bool> (std::bind (&Status::get_zone_open, status, 2)), setFactory<bool> (std::bind (&Status::set_zone_open, status, 2, std::placeholders::_1)), "None", nullptr);
		ep_zone_2->attribute<HassMqtt::Attribute::Types::BINARY_SENSOR> ("is_open", getFactory<float> (std::bind (&Status::get_is_zone_open, status, 2)), "opening", nullptr);

		auto ep_zone_3 = hass->endpoint ("FHCP2mqtt/zone/3");
		ep_zone_3->attribute<HassMqtt::Attribute::Types::SWITCH> ("state", getFactory<bool> (std::bind (&Status::get_zone_open, status, 3)), setFactory<bool> (std::bind (&Status::set_zone_open, status, 3, std::placeholders::_1)), "None", nullptr);
		ep_zone_3->attribute<HassMqtt::Attribute::Types::BINARY_SENSOR> ("is_open", getFactory<float> (std::bind (&Status::get_is_zone_open, status, 3)), "opening", nullptr);

		auto ep_zone_4 = hass->endpoint ("FHCP2mqtt/zone/4");
		ep_zone_4->attribute<HassMqtt::Attribute::Types::SWITCH> ("state", getFactory<bool> (std::bind (&Status::get_zone_open, status, 4)), setFactory<bool> (std::bind (&Status::set_zone_open, status, 4, std::placeholders::_1)), "None", nullptr);
		ep_zone_4->attribute<HassMqtt::Attribute::Types::BINARY_SENSOR> ("is_open", getFactory<float> (std::bind (&Status::get_is_zone_open, status, 4)), "opening", nullptr);

		auto ep_temperature_1 = hass->endpoint ("FHCP2mqtt/temperature/1");
		ep_temperature_1->attribute<HassMqtt::Attribute::Types::SENSOR> ("state", getFactory<float> (std::bind (&Status::get_temperature, status, 1)), "temperature", "measurement", "°C", nullptr);

		auto ep_temperature_2 = hass->endpoint ("FHCP2mqtt/temperature/2");
		ep_temperature_2->attribute<HassMqtt::Attribute::Types::SENSOR> ("state", getFactory<float> (std::bind (&Status::get_temperature, status, 2)), "temperature", "measurement", "°C", nullptr);

		auto ep_temperature_3 = hass->endpoint ("FHCP2mqtt/temperature/3");
		ep_temperature_3->attribute<HassMqtt::Attribute::Types::SENSOR> ("state", getFactory<float> (std::bind (&Status::get_temperature, status, 3)), "temperature", "measurement", "°C", nullptr);

		auto ep_temperature_4 = hass->endpoint ("FHCP2mqtt/temperature/4");
		ep_temperature_4->attribute<HassMqtt::Attribute::Types::SENSOR> ("state", getFactory<float> (std::bind (&Status::get_temperature, status, 4)), "temperature", "measurement", "°C", nullptr);
	} catch (std::exception& e) {
		ESP_LOGE (TAG, "exception found while starting mqtt: %s\n", e.what ());
	}

	ESP_LOGI (TAG, "Start GPIO pin control task...\n");

	TaskControlContainer* container = new TaskControlContainer (status, hass);

	vTaskDelay (pdMS_TO_TICKS (10));
	//create a task for reading analog signals
	xTaskCreatePinnedToCore(
		TaskControl,
		"Analog Read",  // Stack size
		4096,
		(void*)container,  // Task parameter which can modify the task behavior. This must be passed as pointer to void.
		10,                // Priority
		&task_handle_control,     // With task handle we will be able to manipulate with this task.
		CORE_1            // Core on which the task will run
	);
#if 0
	vTaskDelay (pdMS_TO_TICKS (10));

	//create a task for printing heap and task info
	xTaskCreatePinnedToCore(
		DebugTaskInfo,
		"Task Info",  // Stack size
		2048,
		nullptr,  // Task parameter which can modify the task behavior. This must be passed as pointer to void.
		0,                // Priority
		&task_handle_debug,     // With task handle we will be able to manipulate with this task.
		tskNO_AFFINITY            // Core on which the task will run
	);
#endif
	vTaskDelay (pdMS_TO_TICKS (100));

	ESP_LOGI (TAG, "Setup complete.\n");

	vTaskDelay (pdMS_TO_TICKS (10));
}
