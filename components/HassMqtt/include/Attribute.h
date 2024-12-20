#ifndef HASSMQTT_ATTRIBUTE_H
#define HASSMQTT_ATTRIBUTE_H

#include <cstdint>
#include <forward_list>
#include <functional>
#include <array>

#include <cJSON.h>

#include "typedef.h"

namespace HassMqtt {
	class Attribute {
	public:
		typedef enum {
			UNKNOWN = 0,
			NUMBER = 1,
			SENSOR,
			BINARY_SENSOR,
			SWITCH,
			BUTTON,
			SELECT,
			UPDATE,
			CLIMATE,
			LIGHT
		} Types;

		typedef std::function<cJSON*(const char* const endpoint, const char* const attribute, void* user_data)> Getter;
		typedef std::function<void(const cJSON* const, const char* const endpoint, const char* const attribute, void* user_data)> Setter;

	private:
		static constexpr std::array<const char*, 10> _type_names = { "unknown", "number", "sensor", "binary_sensor", "switch", "button", "select", "update", "climate", "light" };

		const Endpoint* const _endpoint;
		const uint32_t _id;
		const Types _type;
		const Getter _getter;
		const Setter _setter;
		char* _name;
		char* _device_class;
		char* _state_class;
		char* _unit;
		char _unique_id[9] = {0,0,0,0,0,0,0,0,0};
		void* _user_data;

	public:
		Attribute (void) = delete;

		Attribute (const Endpoint* const endpoint, const uint32_t& id, const char* name, const Types type, const Getter getter = nullptr, const Setter setter = nullptr, const char* device_class = nullptr, const char* state_class = nullptr, const char* unit = nullptr, void* user_data = nullptr);

		virtual ~Attribute (void);

		void getConfig (cJSON* json) const;

		void publish (cJSON* json) const;

		void set (const cJSON* const json) const;

		inline const char* getName (void) const noexcept {
			return this->_name;
		}

		inline const char* getUniqueId (void) const noexcept {
			return this->_unique_id;
		}

		inline Types getType (void) const noexcept {
			return this->_type;
		}

		inline void* getUserData (void) const noexcept {
			return this->_user_data;
		}

		inline bool hasCommand (void) const noexcept {
			return (this->_setter != nullptr);
		}

		inline bool hasState (void) const noexcept {
			return (this->_getter != nullptr);
		}

		inline const char* getTypeString (void) const {
			return Attribute::_type_names[static_cast<uint8_t>(this->_type)];
		}

		inline uint32_t getId (void) const noexcept {
			return this->_id;
		}

	protected:
		virtual void getTypeConfig (cJSON*) const = 0;
	};

	class AttributeNumber : public Attribute {
	private:
		const float _min = 0;
		const float _max = 100;
		const float _step = 1;
		char* _unit;

	public:
		AttributeNumber (const Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const Setter setter, const char* unit, const float& min = 0, const float& max = 100, const float& step = 1, void* user_data = nullptr);
		~AttributeNumber (void);
		void getTypeConfig (cJSON* json) const;
	};

	class AttributeSensor : public Attribute {
	public:
		AttributeSensor (const Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const char* device_class, const char* state_class, const char* unit, void* user_data = nullptr);
		~AttributeSensor (void);
		void getTypeConfig (cJSON* json) const;
	};

	class AttributeBinarySensor : public Attribute {
	public:
		AttributeBinarySensor (const Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const char* device_class, void* user_data = nullptr);
		~AttributeBinarySensor (void);
		void getTypeConfig (cJSON* json) const;
	};

	class AttributeSwitch : public Attribute {
	public:
		AttributeSwitch (const Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const Setter setter, const char* device_class, void* user_data = nullptr);
		~AttributeSwitch (void);
		void getTypeConfig (cJSON* json) const;
	};

	class AttributeButton : public Attribute {
	public:
		AttributeButton (const Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const Setter setter, const char* device_class, void* user_data = nullptr);
		~AttributeButton (void);
		void getTypeConfig (cJSON* json) const;
	};

	class AttributeSelect : public Attribute {
	private:
		const std::vector<const char*> _options;

	public:
		AttributeSelect (const Endpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, const Setter setter, const std::vector<const char*> options, void* user_data = nullptr);
		~AttributeSelect (void);
		void getTypeConfig (cJSON* json) const;
	};
#if 0
	class HassEndpointAttributeUpdate : public HassEndpointAttribute {
	public:
		HassEndpointAttributeUpdate (const HassEndpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, void* user_data = nullptr);
		~HassEndpointAttributeUpdate (void);
		void getTypeConfig (cJSON* json) const;
	};

	class HassEndpointAttributeClimate : public HassEndpointAttribute {
	public:
		HassEndpointAttributeClimate (const HassEndpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, void* user_data = nullptr);
		~HassEndpointAttributeClimate (void);
		void getTypeConfig (cJSON* json) const;
	};

	class HassEndpointAttributeLight : public HassEndpointAttribute {
	public:
		HassEndpointAttributeLight (const HassEndpoint* const endpoint, const uint32_t& id, const char* name, const Getter getter, void* user_data = nullptr);
		~HassEndpointAttributeLight (void);
		void getTypeConfig (cJSON* json) const;
	};
#endif
}

#include "Endpoint.h"

#endif /* HASSMQTT_ATTRIBUTE_H */
