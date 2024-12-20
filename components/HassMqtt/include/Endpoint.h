#ifndef HASSMQTT_ENDPOINT_H
#define HASSMQTT_ENDPOINT_H

#include <cstdint>
#include <forward_list>
#include <functional>

#include <cJSON.h>

#include "typedef.h"
#include "Discovery.h"
#include "Attribute.h"

namespace HassMqtt {
	class Endpoint {
	private:
		const Discovery* const _hass = nullptr;
		const uint32_t _id;
		const bool _publish_after_set = false;
		std::forward_list<const Attribute*> _attributes;
		char* _name = nullptr;
		char* _endpoint_set = nullptr;
		char _unique_id[9] = {0,0,0,0,0,0,0,0,0};
		uint16_t _attribute_count = 1;

	public:
		Endpoint (void) = delete;

		Endpoint (const Discovery* const hass, const uint32_t& id, const char* const name, const bool& publish_after_set = false);

		~Endpoint (void);

		template<Attribute::Types Type, class... Ts>
		Endpoint* attribute (const char* attribute_name, Ts... args) {
			Attribute* attribute = nullptr;
			const uint32_t id = (static_cast<uint32_t>(this->_id) << 8) | this->_attribute_count++;

			if constexpr (Type == Attribute::Types::NUMBER)
				attribute = new AttributeNumber (this, id, attribute_name, args...);
			else if constexpr (Type == Attribute::Types::SENSOR)
				attribute = new AttributeSensor (this, id, attribute_name, args...);
			else if constexpr (Type == Attribute::Types::BINARY_SENSOR)
				attribute = new AttributeBinarySensor (this, id, attribute_name, args...);
			else if constexpr (Type == Attribute::Types::SWITCH)
				attribute = new AttributeSwitch (this, id, attribute_name, args...);
			else if constexpr (Type == Attribute::Types::BUTTON)
				attribute = new AttributeButton (this, id, attribute_name, args...);
			else if constexpr (Type == Attribute::Types::SELECT)
				attribute = new AttributeSelect (this, id, attribute_name, args...);/*
				else if constexpr (Type == HassEndpointAttribute::Types::UPDATE)
					attribute = new HassEndpointAttributeUpdate (this, id, attribute_name, args...);
			else if constexpr (Type == HassEndpointAttribute::Types::CLIMATE)
				attribute = new HassEndpointAttributeClimate (this, id, attribute_name, args...);
			else if constexpr (Type == HassEndpointAttribute::Types::LIGHT)
				attribute = new HassEndpointAttributeLight (this, id, attribute_name, args...);*/
				else
					throw std::bad_function_call ("");

			this->registerAttribute (attribute);
			return this;
		}

		void publish (void) const;

		inline uint32_t getId (void) const noexcept {
			return this->_id;
		}

		inline const char* getUniqueId (void) const noexcept {
			return this->_unique_id;
		}

		inline const char* getName (void) const noexcept {
			return this->_name;
		}

	private:
		void registerAttribute (const Attribute* const attribute);

		static void set (const char* const topic, const int& topic_len, const int& msg_id, const char* data, const int& data_len, const int& qos, void* user_arg);
	};
}

#endif /* HASSMQTT_ENDPOINT_H */
