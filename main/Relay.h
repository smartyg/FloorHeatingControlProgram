#ifndef RELAY_H
#define RELAY_H

#include <cstdint>
#include <array>

#include <Arduino.h>
#include <esp32-hal-gpio.h>

typedef uint8_t pin_addr_t;

static constexpr pin_addr_t NO_PIN = 255;

template <pin_addr_t ...Pins>
class Relay {
	static constexpr std::array<pin_addr_t, sizeof...(Pins)> _pins = {{Pins...}};

public:
	inline constexpr Relay (void) {
		this->enablePins<Pins...>();
	}

	inline constexpr void on (const std::size_t& idx) const {
		this->setState (idx, LOW);
	}

	inline constexpr void off (const std::size_t& idx) const {
		this->setState (idx, HIGH);
	}

	inline constexpr void allOn (void) const {
		this->setStatePins<Pins...> (LOW);
	}

	inline constexpr void allOff (void) const {
		this->setStatePins<Pins...> (HIGH);
	}

	inline constexpr void setState (const std::size_t& idx, const uint8_t& state) const {
		if (idx < sizeof...(Pins) && this->_pins[idx] < NO_PIN) {
			digitalWrite (this->_pins[idx], state);
		}
	}

	std::size_t inline constexpr numberOfChannels (void) const {
		return sizeof...(Pins);
	}

	inline constexpr bool isValidChannel (const std::size_t& idx) const {
		return (idx < sizeof...(Pins) && this->_pins[idx] < NO_PIN);
	}

	inline constexpr bool isOn (const std::size_t& idx) const {
		if (idx < sizeof...(Pins) && this->_pins[idx] < NO_PIN) {
			auto val = digitalRead (this->_pins[idx]);
			return (val == 0 ? false : true);
		}
		return false;
	}

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
