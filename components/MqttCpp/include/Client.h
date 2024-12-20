/**
 * @file Client.h
 * @brief Defines the Client class for MQTT operations.
 */

#ifndef MQTTCPP_CLIENT_H
#define MQTTCPP_CLIENT_H

#include <forward_list>
#include <mqtt_client.h>

#include "typedef.h"

namespace MqttCpp {
	class Subscription;

	static constexpr const char* const default_broker = "mqtt://127.0.0.1:1883";

	/**
	 * @brief Class representing an MQTT client.
	 */
	class Client {
	private:
		esp_mqtt_client_handle_t _handle; ///< The MQTT client handle.
		bool _connected = false; ///< Whether the client is connected to the broker.
		std::forward_list<const Subscription*> _subscriptions; ///< The list of subscriptions.
		char* _uri = nullptr;

	public:
		/**
		 * @brief Constructs a new Client object.
		 */
		Client (void);

		/**
		 * @brief Destroys the Client object.
		 */
		~Client (void);

		/**
		 * @brief Sets the URI for the MQTT broker.
		 *
		 * @param uri The URI of the MQTT broker.
		 * @return true if the URI was set successfully.
		 * @return false otherwise.
		 */
		bool setUri (const char* uri);

		inline const char* getUri (void) const noexcept {
			return this->_uri;
		}

		/**
		 * @brief Connects to the MQTT broker.
		 *
		 * @return true if the connection was successful.
		 * @return false otherwise.
		 */
		bool connect (void);

		/**
		 * @brief Disconnects from the MQTT broker.
		 *
		 * @return true if the disconnection was successful.
		 * @return false otherwise.
		 */
		bool disconnect (void);

		/**
		 * @brief Publishes a message to a topic.
		 *
		 * @param topic The topic to publish to.
		 * @param msg The message to publish.
		 * @param msg_len The length of the message.
		 * @param qos The Quality of Service level.
		 * @param retain Whether to retain the message.
		 * @param block Whether to block until the message is published.
		 * @return true if the message was published successfully.
		 * @return false otherwise.
		 */
		bool publish (const char* const topic, const char* const msg, const int& msg_len, const int& qos = 1, const bool& retain = false, const bool& block = false);

		/**
		 * @brief Subscribes to a topic.
		 *
		 * @param topic The topic to subscribe to.
		 * @param callback The callback function to call when a message is received.
		 * @param qos The Quality of Service level.
		 * @param user_data User data to pass to the callback function.
		 * @return true if the subscription was successful.
		 * @return false otherwise.
		 */
		bool subscribe (const char* const topic, const Callback callback, const int& qos, void* user_data);

		inline bool isConnected (void) const noexcept {
			return this->_connected;
		}

	private:
		/**
		 * @brief Handles MQTT events.
		 *
		 * @param handler_args User data registered to the event.
		 * @param base Event base for the handler.
		 * @param event_id The ID for the received event.
		 * @param event_data The data for the event.
		 */
		static void eventHandler (void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

		/**
		 * @brief Matches a topic string against a subscription topic.
		 *
		 * @param str1 The subscription topic.
		 * @param str1_len The length of the subscription topic.
		 * @param str2 The message topic.
		 * @param str2_len The length of the message topic.
		 * @return true if the topics match.
		 * @return false otherwise.
		 */
		static bool matchTopic (const char* const str1, const int& str1_len, const char* const str2, const int& str2_len) noexcept;
	};

} // namespace MqttCpp

#endif /* MQTTCPP_CLIENT_H */
