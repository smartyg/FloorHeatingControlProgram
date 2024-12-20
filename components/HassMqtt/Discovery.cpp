#include "Discovery.h"

#include <stdexcept>

#include <cJSON.h>
#include <MqttCpp.h>

#include "Attribute.h"

#include "utils.h"

HassMqtt::Discovery::Discovery (MqttCpp::Client* mqtt, const char* identifier, const char* manufacturer, const char* model, const char* device, const char* sw_version) : _mqtt(mqtt) {
	if (!this->_mqtt->isConnected ()) throw std::runtime_error ("not connected to a MQTT broker.");

	cJSON* json_device = cJSON_CreateObject ();
	cJSON_AddItemToObject (json_device, "identifiers", cJSON_CreateString (identifier));
	cJSON_AddItemToObject (json_device, "manufacturer", cJSON_CreateString (manufacturer));
	cJSON_AddItemToObject (json_device, "model", cJSON_CreateString (model));
	cJSON_AddItemToObject (json_device, "name", cJSON_CreateString (device));
	cJSON_AddItemToObject (json_device, "sw_version", cJSON_CreateString (sw_version));
	this->_json_device = json_device;

	cJSON* json_origin = cJSON_CreateObject ();
	cJSON_AddItemToObject (json_origin, "name", cJSON_CreateString ("aaa"));
	cJSON_AddItemToObject (json_origin, "sw", cJSON_CreateString ("Martijn Goedhart"));
	cJSON_AddItemToObject (json_origin, "url", cJSON_CreateString ("aaa"));
	this->_json_origin = json_origin;

	cJSON* json_availability = cJSON_CreateObject ();
	const char* controller_topic = HassMqtt::utils::makeTopic ('/', device, "controller");
	cJSON_AddItemToObject (json_availability, "topic", cJSON_CreateString (controller_topic));
	cJSON_AddItemToObject (json_availability, "value_template", cJSON_CreateString ("{{ value_json.available }}"));
	this->_json_availability = json_availability;

	for (int i = 0; identifier[i] != '\0'; ++i) {
		this->_id += static_cast<uint8_t>(identifier[i]);
	}

	this->_discoveryTopic = HassMqtt::utils::strdup ("homeassistant");
	HassMqtt::utils::genId (this->_id, this->_unique_id);

	this->_controller = this->endpoint (controller_topic, false);
	this->_controller->attribute<HassMqtt::Attribute::Types::BINARY_SENSOR> ("available", std::bind (&HassMqtt::Discovery::isAvailableJson, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), "connectivity", this);
	free (const_cast<char*>(controller_topic));
}

HassMqtt::Discovery::~Discovery (void) {
	this->_available = false;
	this->_controller->publish ();

	for (HassMqtt::Endpoint* ep : this->_endpoints) {
		delete ep;
	}
	this->_endpoints.clear ();

	cJSON_Delete (const_cast<cJSON*>(this->_json_device));
	cJSON_Delete (const_cast<cJSON*>(this->_json_origin));
	cJSON_Delete (const_cast<cJSON*>(this->_json_availability));
}

HassMqtt::Endpoint* HassMqtt::Discovery::endpoint (const char* endpoint_name, const bool& publish_after_set) {
	HassMqtt::Endpoint* ep = new HassMqtt::Endpoint (this, (static_cast<uint32_t>(this->_id++) << 8) | this->_enpoint_count, endpoint_name, publish_after_set);
	this->_endpoints.push_front (ep);
	return ep;
}

void HassMqtt::Discovery::publishAll (void) const {
	for (const HassMqtt::Endpoint* ep : this->_endpoints) {
		ep->publish ();
	}
}

void HassMqtt::Discovery::publishAttributeConfig (const char* const component, const char* const object_id, const char* const attribute, cJSON* json, const uint8_t& qos) const {
	cJSON_AddItemToObject (json, "device", cJSON_Duplicate (this->_json_device, 1));
	cJSON_AddItemToObject (json, "origin", cJSON_Duplicate (this->_json_origin, 1));
	cJSON_AddItemToObject (json, "availability", cJSON_Duplicate (this->_json_availability, 1));

	const char* json_string = cJSON_Print (json);
	int json_string_len = 0;
	while (json_string[json_string_len] != '\0') ++json_string_len;

	const char* const topic = HassMqtt::utils::makeTopic ('/', this->_discoveryTopic, component, object_id, attribute, "config");

	this->_mqtt->publish (topic, json_string, json_string_len, qos, true, false);
}

void HassMqtt::Discovery::publishEndpoint (const char* const topic, const char* const data, const uint8_t& qos) const {
	if (topic == nullptr || data == nullptr) return;
	int data_len = 0;
	while (data[data_len] != '\0') ++data_len;

	this->_mqtt->publish (topic, data, data_len, qos, false, false);
}

void HassMqtt::Discovery::subscribe (const char* topic, const MqttCpp::Callback func, const uint8_t& qos, void* user_data) const {
	if (!this->_mqtt->subscribe (topic, func, qos, user_data)) throw std::runtime_error ("Can not register topic to mqtt broker.");
}
