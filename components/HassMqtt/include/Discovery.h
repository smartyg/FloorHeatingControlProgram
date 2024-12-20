#ifndef HASSMQTT_DISCOVERY_H
#define HASSMQTT_DISCOVERY_H

#include <cstdint>
#include <forward_list>

#include <cJSON.h>
#include <MqttCpp.h>

#include "typedef.h"
#include "Endpoint.h"

namespace HassMqtt {
	class Discovery {
	private:
		mutable MqttCpp::Client* _mqtt;
		std::forward_list<Endpoint*> _endpoints;
		const cJSON* _json_device = nullptr;
		const cJSON* _json_origin = nullptr;
		const cJSON* _json_availability = nullptr;
		bool _available = true;
		char* _discoveryTopic = nullptr;
		uint32_t _id = 0;
		uint16_t _enpoint_count = 1;
		char _unique_id[9] = {0,0,0,0,0,0,0,0,0};
		Endpoint* _controller;

	public:
		Discovery (void) = delete;

		Discovery (MqttCpp::Client* mqtt, const char* identifier, const char* manufacturer, const char* model, const char* device, const char* sw_version);

		~Discovery (void);

		Endpoint* endpoint (const char* endpoint_name, const bool& publish_after_set = false);
		void publishAll (void) const;
		void publishAttributeConfig (const char* const component, const char* const object_id, const char* const attribute, cJSON* json, const uint8_t& qos = 2) const;
		void publishEndpoint (const char* const topic, const char* const data, const uint8_t& qos = 1) const;
		void subscribe (const char* topic, const MqttCpp::Callback func, const uint8_t& qos, void* user_data) const;

		inline bool isAvailable (void) const noexcept {
			return this->_available;
		}

		inline cJSON* isAvailableJson (const char* const, const char* const, void*) const {
			if (this->isAvailable ())
				return cJSON_CreateTrue ();
			else
				return cJSON_CreateFalse ();
		}

		inline uint32_t getId (void) const noexcept {
			return this->_id;
		}

		inline const char* getUniqueId (void) const noexcept {
			return this->_unique_id;
		}

		inline const Endpoint* getControllerEndpoint (void) const noexcept {
			return this->_controller;
		}
	};
}

#endif /* HASSMQTT_DISCOVERY_H */
