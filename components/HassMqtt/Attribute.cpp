#include "Attribute.h"

#include <cJSON.h>

#include "Endpoint.h"
#include "utils.h"

// *** General ***
HassMqtt::Attribute::Attribute (const HassMqtt::Endpoint* const endpoint, const uint32_t& id, const char* name, const Types type, const Getter getter, const Setter setter, const char* device_class, const char* state_class, const char* unit, void* user_data) : _endpoint(endpoint), _id(id), _type(type), _getter(getter), _setter(setter), _user_data(user_data) {
	this->_name = HassMqtt::utils::strdup (name);
	this->_device_class = HassMqtt::utils::strdup (device_class);
	this->_state_class = HassMqtt::utils::strdup (state_class);
	this->_unit = HassMqtt::utils::strdup (unit);
	HassMqtt::utils::genId (this->_id, this->_unique_id);
}

HassMqtt::Attribute::~Attribute (void) {
	free (this->_name);
	free (this->_device_class);
		free (this->_state_class);
		free (this->_unit);
		this->_name = nullptr;
		this->_device_class = nullptr;
		this->_state_class = nullptr;
		this->_unit = nullptr;
		this->_user_data = nullptr;

}

void HassMqtt::Attribute::getConfig (cJSON* json) const {
	cJSON_AddItemToObject (json, "object_id", cJSON_CreateString (this->_endpoint->getUniqueId ()));
	cJSON_AddItemToObject (json, "unique_id", cJSON_CreateString (this->_unique_id));
	cJSON_AddItemToObject (json, "name", cJSON_CreateString (this->_name));
	char* value_template = HassMqtt::utils::makeTopic ('\0', "{{ value_json.", this->_name, " }}");
	if (this->hasState ()) cJSON_AddItemToObject (json, "value_template", cJSON_CreateString (value_template));
	free (value_template);
	if (this->_device_class != nullptr) cJSON_AddItemToObject (json, "device_class", cJSON_CreateString (this->_device_class));
	if (this->_state_class != nullptr) cJSON_AddItemToObject (json, "state_class", cJSON_CreateString (this->_state_class));
	if (this->_unit != nullptr) cJSON_AddItemToObject (json, "unit_of_measurement", cJSON_CreateString (this->_unit));

	this->getTypeConfig (json);
}

void HassMqtt::Attribute::publish (cJSON* json) const {
	if (!this->hasState ()) return;
	cJSON* value = this->_getter (this->_endpoint->getName (), this->_name, this->_user_data);
	cJSON_AddItemToObject (json, this->_name, value);
}

void HassMqtt::Attribute::set (const cJSON* const json) const {
	if (!this->hasCommand ()) return;
	this->_setter (json, this->_endpoint->getName (), this->_name, this->_user_data);
}

// *** Number ***
HassMqtt::AttributeNumber::AttributeNumber (const HassMqtt::Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const Setter setter, const char* unit, const float& min, const float& max, const float& step, void* user_data) : HassMqtt::Attribute (endpoint, id, name, NUMBER, getter, setter, nullptr, nullptr, unit, user_data), _min(min), _max(max), _step(step) {}

HassMqtt::AttributeNumber::~AttributeNumber (void) {}

void HassMqtt::AttributeNumber::getTypeConfig (cJSON* json) const {
	cJSON_AddItemToObject (json, "min", cJSON_CreateNumber (this->_min));
	cJSON_AddItemToObject (json, "max", cJSON_CreateNumber (this->_max));
	cJSON_AddItemToObject (json, "step", cJSON_CreateNumber (this->_step));
}

// *** Sensor ***
HassMqtt::AttributeSensor::AttributeSensor (const HassMqtt::Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const char* device_class, const char* state_class, const char* unit, void* user_data) : HassMqtt::Attribute (endpoint, id, name, SENSOR, getter, nullptr, device_class, state_class, unit, user_data) {}
HassMqtt::AttributeSensor::~AttributeSensor (void) {}

void HassMqtt::AttributeSensor::getTypeConfig (cJSON* json) const {
		(void)json;
	}

// *** Binary sensor ***
HassMqtt::AttributeBinarySensor::AttributeBinarySensor (const HassMqtt::Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const char* device_class, void* user_data) : HassMqtt::Attribute (endpoint, id, name, BINARY_SENSOR, getter, nullptr, device_class, nullptr, nullptr, user_data) {}

HassMqtt::AttributeBinarySensor::~AttributeBinarySensor (void) {}

void HassMqtt::AttributeBinarySensor::getTypeConfig (cJSON* json) const {
	cJSON_AddItemToObject (json, "payload_off", cJSON_CreateFalse ());
	cJSON_AddItemToObject (json, "payload_on", cJSON_CreateTrue ());
}

// *** Switch ***
HassMqtt::AttributeSwitch::AttributeSwitch (const HassMqtt::Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const Setter setter, const char* device_class, void* user_data) : HassMqtt::Attribute (endpoint, id, name, SWITCH, getter, setter, device_class, nullptr, nullptr, user_data) {}
HassMqtt::AttributeSwitch::~AttributeSwitch (void) {}

void HassMqtt::AttributeSwitch::getTypeConfig (cJSON* json) const {
	cJSON_AddItemToObject (json, "payload_off", cJSON_CreateFalse ());
	cJSON_AddItemToObject (json, "payload_on", cJSON_CreateTrue ());
}

// *** Button ***
HassMqtt::AttributeButton::AttributeButton (const HassMqtt::Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const Setter setter, const char* device_class, void* user_data) : HassMqtt::Attribute (endpoint, id, name, BINARY_SENSOR, getter, setter, device_class, nullptr, nullptr, user_data) {}
HassMqtt::AttributeButton::~AttributeButton (void) {}

void HassMqtt::AttributeButton::getTypeConfig (cJSON* json) const {
	(void)json;
}

// *** Select ***
HassMqtt::AttributeSelect::AttributeSelect (const HassMqtt::Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const Setter setter, const std::vector<const char*> options, void* user_data) : HassMqtt::Attribute (endpoint, id, name, SWITCH, getter, setter, nullptr, nullptr, nullptr, user_data), _options(options) {}

HassMqtt::AttributeSelect::~AttributeSelect (void) {}

void HassMqtt::AttributeSelect::getTypeConfig (cJSON* json) const {
	cJSON* json_array = cJSON_CreateArray ();
	for (const char* o : this->_options)
		cJSON_AddItemToArray (json_array, cJSON_CreateString (o));
	cJSON_AddItemToObject (json, "options", json_array);
}

// *** Update ***

// *** Climate ***

// *** Light ***
