#include "TaskControl.h"

#include <array>
#include <cstdio>
#include <bsp/arduino-nano-esp32.h>

// Local components
#include "DS18B20.h"

#include "Status.h"
#include "Relay.h"

#define COVERT_BOOL_TO_HIGH_LOW(b) ((b) == true ? 1 : 0)
#define COVERT_HIGH_LOW_TO_BOOL(v) ((v) == 0 ? false : true)

//static constexpr uint8_t LED_RED = PIN_D14;
//static constexpr uint8_t LED_GREEN = PIN_D15;
//static constexpr uint8_t LED_BLUE = PIN_D16;

static constexpr uint8_t PIN_RELAY_CH1 = PIN_D10;
static constexpr uint8_t PIN_RELAY_CH2 = PIN_D9;
static constexpr uint8_t PIN_RELAY_CH3 = PIN_D8;
static constexpr uint8_t PIN_RELAY_CH4 = PIN_D7;
static constexpr uint8_t PIN_RELAY_CH5 = PIN_D6;
static constexpr uint8_t PIN_RELAY_CH6 = NO_PIN;
static constexpr uint8_t PIN_RELAY_CH7 = NO_PIN;
static constexpr uint8_t PIN_RELAY_CH8 = NO_PIN;

static constexpr uint8_t PIN_DISPLAY_SDA = PIN_A2;
static constexpr uint8_t PIN_DISPLAY_SCL = PIN_A3;

static constexpr uint8_t PIN_SENSOR_CH1 = PIN_D5;
static constexpr uint8_t PIN_SENSOR_CH2 = PIN_D4;
static constexpr uint8_t PIN_SENSOR_CH3 = PIN_D3;
static constexpr uint8_t PIN_SENSOR_CH4 = PIN_D2;
static constexpr uint8_t PIN_SENSOR_CH5 = PIN_A0;

static constexpr uint8_t PIN_SENSOR_TEMP = PIN_A1;

static constexpr uint8_t LCD_COLUMNS = 20;
static constexpr uint8_t LCD_LINES = 4;

static constexpr DS18B20::addr_t Probes[] = {
	0x2006244062b61128, // verdeler
	0x3300000034d56f28, // temp 1
	0xf2000000573b6728, // temp 2
	0xd70000005cf02e28, // casing 1
	0x2100000044363f28  // casing 2
};

void TaskControl (void* pvParameters) {  // This is a task.
	Relay<PIN_RELAY_CH1, PIN_RELAY_CH2, PIN_RELAY_CH3, PIN_RELAY_CH4, PIN_RELAY_CH5, PIN_RELAY_CH6, PIN_RELAY_CH7, PIN_RELAY_CH8> relay;

	DS18B20 temperature_sensors (PIN_SENSOR_TEMP);

	Status *s = static_cast<TaskControlContainer*>(pvParameters)->status;
	HassMqtt::Discovery *hass = static_cast<TaskControlContainer*>(pvParameters)->hass;

	vTaskDelay (pdMS_TO_TICKS (100));
	relay.allOff ();

	pinMode (PIN_SENSOR_CH1, INPUT);
	pinMode (PIN_SENSOR_CH2, INPUT);
	pinMode (PIN_SENSOR_CH3, INPUT);
	pinMode (PIN_SENSOR_CH4, INPUT);
	pinMode (PIN_SENSOR_CH5, INPUT);

	int num_devs = temperature_sensors.scanAndSave<10> ();
	printf("Found %d devices\n", num_devs);
	const auto addrs = temperature_sensors.getAddresses ();
	for (int i = 0; i < num_devs; i += 1) {
		const auto temp = temperature_sensors.getTemperature (addrs[i]);
		printf ("\t%d: 0x%llx: %f\n", i, addrs[i], temp / 16.0);
	}
	putchar ('\n');
	/*
	for (const auto& addr : temperature_sensors) {
		const auto temp = temperature_sensors.getTemperature (addr);
		printf ("\t0x%llx: %d\n", addr, temp);
		printf ("\t0x%llx\n", addrs[i]);
	}
	*/

	for (;;) {

		// read temperature
		for (int i = 0; i < sizeof(Probes); ++i) {
			s->set_temperature(i, temperature_sensors.getTemperature (Probes[i]) / 16.0);
		}

		if (s->is_mode_auto ()) {
			if (s->get_temperature (0) > (s->get_target_temperature () + s->get_target_temperature_range ())) {
				s->set_inlet_open (false);
				relay.off (0);
			} else if (s->get_temperature (0) < (s->get_target_temperature () - s->get_target_temperature_range ())) {
				s->set_inlet_open (true);
				relay.on (0);
			}
		} else {
			relay.setState (0, COVERT_BOOL_TO_HIGH_LOW (s->get_inlet_open ()));
		}

		for (std::size_t idx = 1; idx < relay.numberOfChannels (); ++idx) {
			relay.setState (idx, COVERT_BOOL_TO_HIGH_LOW(s->get_zone_open (idx)));
		}

		s->set_is_inlet_open (COVERT_HIGH_LOW_TO_BOOL (digitalRead (PIN_RELAY_CH1)));
		if constexpr (1 < relay.numberOfChannels () && relay.isValidChannel (1))
			s->set_is_zone_open (1, relay.isOn (1));
		if constexpr (2 < relay.numberOfChannels () && relay.isValidChannel (2))
			s->set_is_zone_open (2, COVERT_HIGH_LOW_TO_BOOL (digitalRead (PIN_RELAY_CH3)));
		if constexpr (3 < relay.numberOfChannels () && relay.isValidChannel (3))
			s->set_is_zone_open (3, COVERT_HIGH_LOW_TO_BOOL (digitalRead (PIN_RELAY_CH4)));
		if constexpr (4 < relay.numberOfChannels () && relay.isValidChannel (4))
			s->set_is_zone_open (4, COVERT_HIGH_LOW_TO_BOOL (digitalRead (PIN_RELAY_CH5)));
		if constexpr (5 < relay.numberOfChannels () && relay.isValidChannel (5))
			s->set_is_zone_open (5, COVERT_HIGH_LOW_TO_BOOL (digitalRead (PIN_RELAY_CH6)));
		if constexpr (6 < relay.numberOfChannels () && relay.isValidChannel (6))
			s->set_is_zone_open (6, COVERT_HIGH_LOW_TO_BOOL (digitalRead (PIN_RELAY_CH7)));
		if constexpr (7 < relay.numberOfChannels () && relay.isValidChannel (7))
			s->set_is_zone_open (7, COVERT_HIGH_LOW_TO_BOOL (digitalRead (PIN_RELAY_CH8)));

		vTaskDelay (pdMS_TO_TICKS (100));

		//hass->publishAll ();

		// update LCD
		#if 0
		lcd.clear ();
		for (int i = 0; i < LCD_LINES; ++i) {
			lcd.setCursor (0, i);
			s->generate_msg (lcd_buffer[i].data (), LCD_COLUMNS, i);
			lcd.print (lcd_buffer[i].data ());
	}
	#endif

	vTaskDelay (pdMS_TO_TICKS (1000));
	}

}

