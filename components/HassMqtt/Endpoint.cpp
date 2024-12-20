#include "Endpoint.h"

#include <cJSON.h>

#include "Discovery.h"
#include "Attribute.h"
#include "utils.h"

HassMqtt::Endpoint::Endpoint (const HassMqtt::Discovery* const hass, const uint32_t& id, const char* const name, const bool& publish_after_set) : _hass(hass), _id(id), _publish_after_set(publish_after_set) {
	this->_name = HassMqtt::utils::strdup (name);

	//subscribe to endpoint_name + /set
	this->_endpoint_set = HassMqtt::utils::makeTopic ('/', this->_name, "set");
	HassMqtt::utils::genId (this->_id, this->_unique_id);

	this->_hass->subscribe (this->_endpoint_set, &HassMqtt::Endpoint::set, 0, this);
}

HassMqtt::Endpoint::~Endpoint (void) {
	for (const HassMqtt::Attribute* attr : this->_attributes) {
		delete attr;
	}
	this->_attributes.clear ();
	free (this->_name);
	free (this->_endpoint_set);
	this->_name = nullptr;
	this->_endpoint_set = nullptr;
}

void HassMqtt::Endpoint::publish (void) const {
	cJSON* json = cJSON_CreateObject ();
	for (const HassMqtt::Attribute* attr : this->_attributes) {
		attr->publish (json);
	}
	const char* const data = cJSON_Print (json);
	cJSON_Delete (json);
	this->_hass->publishEndpoint (this->_name, data, 0);
}

void HassMqtt::Endpoint::registerAttribute (const HassMqtt::Attribute* const attribute) {
	if (attribute != nullptr) {
		this->_attributes.push_front (attribute);

		cJSON* json = cJSON_CreateObject ();

		if (attribute->hasCommand ()) cJSON_AddItemToObject (json, "command_topic", cJSON_CreateString (this->_endpoint_set));
		if (attribute->hasState ()) cJSON_AddItemToObject (json, "state_topic", cJSON_CreateString (this->_name));

		attribute->getConfig (json);

		this->_hass->publishAttributeConfig (attribute->getTypeString (), this->getUniqueId (), attribute->getName (), json, 1);

		cJSON_Delete (json);
	}
}

void HassMqtt::Endpoint::set (const char* const topic, const int& topic_len, const int& msg_id, const char* data, const int& data_len, const int& qos, void* user_arg) {
	(void)topic;
	(void)topic_len;
	(void)msg_id;
	(void)qos;

	const cJSON* json = cJSON_ParseWithLength (data, data_len);

	const HassMqtt::Endpoint* ep = static_cast<HassMqtt::Endpoint*>(user_arg);

	for (const HassMqtt::Attribute* attr : ep->_attributes) {
		const char* const attribute_name = attr->getName ();
		if (cJSON_HasObjectItem (json, attribute_name) == 1) {
			const cJSON* const attribute_json = cJSON_GetObjectItem (json, attribute_name);
			attr->set (attribute_json);
		}
	}

	if (ep->_publish_after_set)
		ep->publish ();
}
