/**
 * @file Subscription.h
 * @brief Defines the Subscription structure used for MQTT message callbacks.
 */

#ifndef MQTTCPP_SUBSCRIPTIONT_H
#define MQTTCPP_SUBSCRIPTIONT_H

#include <cstdint>
#include <MqttCpp.h>

namespace MqttCpp {

	/**
	 * @brief Structure to hold subscription details.
	 */
	struct Subscription {
		const char* _topic = nullptr; ///< The topic to subscribe to.
		uint16_t _topic_len = 0; ///< The length of the topic.
		const Callback _callback; ///< The callback function for the subscription.
		const uint8_t _qos = 2; ///< The Quality of Service level.
		void* _user_data = nullptr; ///< User data to pass to the callback function.

		/**
		 * @brief Constructs a new Subscription object.
		 *
		 * @param topic The topic to subscribe to.
		 * @param callback The callback function for the subscription.
		 * @param qos The Quality of Service level.
		 * @param user_data User data to pass to the callback function.
		 */
		Subscription (const char* const topic, const Callback callback, const uint8_t& qos = 2, void* user_data = nullptr) : _callback(callback), _qos(qos), _user_data(user_data) {
			this->_topic = strdup(topic);
			this->_topic_len = strlen(this->_topic);
		}

		/**
		 * @brief Destroys the Subscription object.
		 */
		~Subscription (void) {
			free (const_cast<char*>(this->_topic));
			this->_topic = nullptr;
			this->_user_data = nullptr;
		}
	};

} // namespace MqttCpp

#endif /* MQTTCPP_SUBSCRIPTIONT_H */
