/**
 * @file typedef.h
 * @brief Defines type aliases for the MqttCpp library.
 */

#ifndef MQTTCPP_TYPEDEF_H
#define MQTTCPP_TYPEDEF_H

#include <functional>

namespace MqttCpp {
	inline namespace types {
		/**
		 * @brief Type alias for the MQTT message callback function.
		 *
		 * This function is called when a message is received on a subscribed topic.
		 *
		 * @param topic The topic the message was published on.
		 * @param topic_len The length of the topic string.
		 * @param msg_id The message ID.
		 * @param data The message payload.
		 * @param total_data_len The total length of the message payload.
		 * @param qos The Quality of Service level.
		 * @param user_data User data passed to the callback function.
		 */
		typedef std::function<void(const char* const topic, const int& topic_len, const int& msg_id, const char* const data, const int& total_data_len, const int& qos, void* user_data)> Callback;
	}
}

#endif /* MQTTCPP_TYPEDEF_H */
