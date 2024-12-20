#include "DS18B20.h"

#include <stdio.h>
#include <onewire.h>

void DS18B20::setDevice (const addr_t* const id_codes, const uint8_t& num) {
	if (this->_malloc) heap_caps_free (this->_rom_codes);
	this->_rom_codes = const_cast<addr_t*>(id_codes);
	this->_size = num;
	this->_malloc = false;
}

int16_t DS18B20::getNext (void) {
	if (this->_it == nullptr || this->_it == this->cend ()) {
		this->_it = this->cbegin ();
	} else {
		this->_it++;
	}
	return this->getTemperature (*(this->_it));
}

int16_t DS18B20::getTemperature (const addr_t& romcode) {
	//if (this->_reset_needed) this->resetDevice (true);
	this->resetDevice (true);
	ow_reset (&(this->_ow));
	ow_send (&(this->_ow), OW_MATCH_ROM);
	for (int b = 0; b < 64; b += 8) {
		ow_send (&(this->_ow), romcode >> b);
	}
	ow_send (&(this->_ow), DS18B20_READ_SCRATCHPAD);
	int16_t temp = 0;
	temp = (ow_read (&(this->_ow)) | (ow_read (&(this->_ow)) << 8)) >> 4;
	return temp;
}

void DS18B20::resetDevice (const bool& skip_rom) {
	ow_reset (&(this->_ow));
	if (skip_rom)
		ow_send (&(this->_ow), OW_SKIP_ROM);
	ow_send (&(this->_ow), DS18B20_CONVERT_T);
	this->_reset_needed = false;
}

