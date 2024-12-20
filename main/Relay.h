/**
 * @file Relay.h
 * @brief Defines the Relay class template for managing relay channels.
 */

#ifndef RELAY_H
#define RELAY_H

#include <cstdint>
#include <array>

#include <Arduino.h>
#include <esp32-hal-gpio.h>

typedef uint8_t pin_addr_t;

static constexpr pin_addr_t NO_PIN = 255;

/**
 * @class Relay
 * @brief Template class for managing relay channels.
 * @tparam Pins The GPIO pins connected to the relays.
 */
template <pin_addr_t ...Pins>
class Relay {
	static constexpr std::array<pin_addr_t, sizeof...(Pins)> _pins = {{Pins...}};

public:
	/**
	 * @brief Constructs a Relay object and initializes the GPIO pins.
	 */
	inline constexpr Relay (void) {
		this->enablePins<Pins...>();
	}

	/**
	 * @brief Turns on the relay at the specified index.
	 * @param idx The index of the relay to turn on.
	 */
	inline constexpr void on (const std::size_t& idx) const {
		this->setState (idx, LOW);
	}

	/**
	 * @brief Turns off the relay at the specified index.
	 * @param idx The index of the relay to turn off.
	 */
	inline constexpr void off (const std::size_t& idx) const {
		this->setState (idx, HIGH);
	}

	/**
	 * @brief Turns on all relays.
	 */
	inline constexpr void allOn (void) const {
		this->setStatePins<Pins...> (LOW);
	}

	/**
	 * @brief Turns off all relays.
	 */
	inline constexpr void allOff (void) const {
		this->setStatePins<Pins...> (HIGH);
	}

	/**
	 * @brief Sets the state of the relay at the specified index.
	 * @param idx The index of the relay.
	 * @param state The state to set (LOW or HIGH).
	 */
	inline constexpr void setState (const std::size_t& idx, const uint8_t& state) const {
		if (idx < sizeof...(Pins) && this->_pins[idx] < NO_PIN) {
			digitalWrite (this->_pins[idx], state);
		}
	}

	/**
	 * @brief Gets the number of relay channels.
	 * @return The number of relay channels.
	 */
	std::size_t inline constexpr numberOfChannels (void) const {
		return sizeof...(Pins);
	}

	/**
	 * @brief Checks if the specified index is a valid relay channel.
	 * @param idx The index to check.
	 * @return True if the index is valid, false otherwise.
	 */
	inline constexpr bool isValidChannel (const std::size_t& idx) const {
		return (idx < sizeof...(Pins) && this->_pins[idx] < NO_PIN);
	}

	/**
	 * @brief Checks if the relay at the specified index is on.
	 * @param idx The index of the relay to check.
	 * @return True if the relay is on, false otherwise.
	 */
	inline constexpr bool isOn (const std::size_t& idx) const {
		if (idx < sizeof...(Pins) && this->_pins[idx] < NO_PIN) {
			auto val = digitalRead (this->_pins[idx]);
			return (val == 0 ? false : true);
		}
		return false;
	}

	/**
	 * @brief Gets the GPIO pin for the specified index.
	 * @param idx The index to get the GPIO pin for.
	 * @return The GPIO pin number.
	 */
	inline constexpr pin_addr_t getGPIO (const std::size_t& idx) const {
		if (idx < sizeof...(Pins)) {
			return this->_pins[idx];
		}
		return NO_PIN;
	}

private:
	template<pin_addr_t P, pin_addr_t ...Ps>
	inline constexpr void enablePins (void) const {
		this->enablePinsReal<P>();
		if constexpr (sizeof...(Ps) > 0) {
			this->enablePins<Ps...> ();
		}
	}

	template<pin_addr_t P>
	inline constexpr void enablePinsReal (void) const {
		if constexpr (P < NO_PIN) {
			pinMode (P, OUTPUT);
		}
	}

	template<pin_addr_t P, pin_addr_t ...Ps>
	inline constexpr void setStatePins (const uint8_t& state) const {
		this->setStatePinsReal<P>(state);
		if constexpr (sizeof...(Ps) > 0) {
			this->setStatePins<Ps...> (state);
		}
	}

	template<pin_addr_t P>
	inline constexpr void setStatePinsReal (const uint8_t& state) const {
		if constexpr (P < NO_PIN) {
			digitalWrite (P, state);
		}
	}
};

#endif /* RELAY_H */
