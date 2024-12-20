#ifndef DS18B20_H
#define DS18B20_H

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_heap_caps.h>
#include <onewire.h>

class DS18B20 {
public:
	typedef uint64_t addr_t;

private:
	OW _ow;
	const uint8_t _gpio_pin;
	bool _reset_needed = true;
	addr_t* _rom_codes = nullptr;
	size_t _size = 0;
	bool _malloc = false;
	const addr_t* _it = nullptr;

public:
	DS18B20 (const uint8_t gpio_pin) : _gpio_pin(gpio_pin) {
		ow_init (&(this->_ow), this->_gpio_pin);
	}

	~DS18B20 (void) {
		if (this->_malloc) heap_caps_free (this->_rom_codes);
		this->_rom_codes = nullptr;
		this->_it = nullptr;
	}

	template<std::size_t N>
	int scan (addr_t* results) {
		if (ow_reset (&(this->_ow))) {
			this->_reset_needed = true;
			return ow_romsearch (&(this->_ow), results, N, OW_SEARCH_ROM);
		}
		return 0;
	}

	template<std::size_t N>
	int scanAndSave (void) {
		if (this->_malloc) heap_caps_free (this->_rom_codes);
		this->_rom_codes = static_cast<addr_t*>(heap_caps_malloc (N * sizeof (addr_t), MALLOC_CAP_8BIT));
		this->_malloc = true;
		this->_reset_needed = true;
		this->_size = this->scan<N> (this->_rom_codes);
		return this->_size;
	}

	void setDevice (const addr_t* const id_codes, const uint8_t& num);
	int16_t getNext (void);
	int16_t getTemperature (const addr_t& romcode);

	const addr_t* begin (void) const noexcept {
		return this->_rom_codes;
	}

	const addr_t* cbegin (void) const noexcept {
		return this->_rom_codes;
	}

	const addr_t* end (void) const noexcept {
		return (this->_rom_codes + this->_size);
	}

	const addr_t* cend (void) const noexcept {
		return (this->_rom_codes + this->_size);
	}

	addr_t* getAddresses (void) const noexcept {
		return this->_rom_codes;
	}

	size_t numAddresses (void) const noexcept {
		return this->_size;
	}

private:
	void resetDevice (const bool& skip_rom = true);
};

#endif /* DS18B20_H */
