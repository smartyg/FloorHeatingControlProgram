/*
 * Class Status
 */

#ifndef STATUS_H
#define STATUS_H

#include <mutex>
#include <atomic>

class Status {
public:
	enum class ControlMode: std::uint8_t {
		AUTOMATIC = 1,
		MANUAL = 0
	};
private:
	typedef uint16_t int_temp_t;

	volatile std::atomic<std::uint32_t> _data[4] = {0, 0, 0, 0};
	volatile std::atomic<std::uintptr_t> _msg[4] = {0, 0, 0, 0};
	mutable std::mutex _m_msg;

	#define GET_BITS(d, n, s) (((d) >> (n)) & (s))
	#define SET_BITS(d, n, s, v) (d = (((d) & (~(static_cast<uint32_t>(s) << (n)))) | (static_cast<uint32_t>(v) << (n))))

	#define CONTROL_MODE(d) (static_cast<ControlMode>(GET_BITS(d[0], 31, 1)))
	#define TARGET_TEMPERATURE(d) (static_cast<int_temp_t>(GET_BITS(d[0], 22, 511)))
	#define TARGET_TEMPERATURE_RANGE(d) (static_cast<int_temp_t>(GET_BITS(d[1], 22, 511)))
	#define TEMPERATURE(d, z) (static_cast<int_temp_t>(GET_BITS(d[z & 3], (9 * ((z >> 2) & 1)), 511)))
	#define ZONE_OPEN(d, z) (static_cast<bool>(GET_BITS(d[z & 3], (18 + ((z >> 2) & 1)), 1)))
	#define IS_ZONE_OPEN(d, z) (static_cast<bool>(GET_BITS(d[z & 3], (20 + ((z >> 2) & 1)), 1)))

	#define SET_CONTROL_MODE(d, v) (SET_BITS(d[0], 31, 1, v))
	#define SET_TARGET_TEMPERATURE(d, v) (SET_BITS(d[0], 22, 511, v))
	#define SET_TARGET_TEMPERATURE_RANGE(d, v) (SET_BITS(d[1], 22, 511, v))
	#define SET_TEMPERATURE(d, z, v) (SET_BITS(d[z & 3], (9 * ((z >> 2) & 1)), 511, v))
	#define SET_ZONE_OPEN(d, z, v) (SET_BITS(d[z & 3], (18 + ((z >> 2) & 1)), 1, v))
	#define SET_IS_ZONE_OPEN(d, z, v) (SET_BITS(d[z & 3], (20 + ((z >> 2) & 1)), 1, v))

public:
	Status (void) {}

	inline constexpr bool is_mode_auto (void) const noexcept { return (CONTROL_MODE(this->_data) == ControlMode::AUTOMATIC); }
	inline constexpr bool is_mode_manual (void) const noexcept { return (CONTROL_MODE(this->_data) == ControlMode::MANUAL); }
	inline constexpr bool set_mode (const ControlMode& m) noexcept { SET_CONTROL_MODE(this->_data, m); return true; }
	inline constexpr bool set_target_temperature (const float& t) noexcept { SET_TARGET_TEMPERATURE(this->_data, float_to_internal_temp (t)); return true; }
	inline constexpr float get_target_temperature (void) const noexcept { return internal_temp_to_float (TARGET_TEMPERATURE (this->_data)); }
	inline constexpr bool set_target_temperature_range (const float& t) noexcept { SET_TARGET_TEMPERATURE_RANGE(this->_data, float_to_internal_temp (t)); return true; }
	inline constexpr float get_target_temperature_range (void) const noexcept { return internal_temp_to_float (TARGET_TEMPERATURE_RANGE (this->_data)); }
	inline constexpr bool get_inlet_open (void) const noexcept { return ZONE_OPEN(this->_data, 0); }
	inline constexpr bool set_inlet_open (const bool& open) noexcept { SET_ZONE_OPEN(this->_data, 0, open); return true; }
	inline constexpr bool get_is_inlet_open (void) const noexcept { return IS_ZONE_OPEN(this->_data, 0); }
	inline constexpr void set_is_inlet_open (const bool& open) noexcept { SET_IS_ZONE_OPEN(this->_data, 0, open); }
	inline constexpr bool get_zone_open (const uint8_t& zone) const noexcept { return ZONE_OPEN(this->_data, zone); }
	inline constexpr bool set_zone_open (const uint8_t& zone, const bool& open) noexcept { SET_ZONE_OPEN(this->_data, zone, open); return true; }
	inline constexpr bool get_is_zone_open (const uint8_t& zone) const noexcept { return IS_ZONE_OPEN(this->_data, zone); }
	inline constexpr void set_is_zone_open (const uint8_t& zone, const bool open) noexcept { SET_IS_ZONE_OPEN(this->_data, zone, open); }
	inline constexpr float get_temperature (const uint8_t& idx) const noexcept { return TEMPERATURE(this->_data, idx); }
	inline constexpr void set_temperature (const uint8_t& idx, const float& temperature) noexcept { SET_TEMPERATURE(this->_data, idx, float_to_internal_temp(temperature)); }
	//inline constexpr float get_temperature (void) const noexcept { return TEMPERATURE(this->_data, 0); }
	//inline constexpr void set_temperature (const float& temperature) noexcept { SET_TEMPERATURE(this->_data, 0, float_to_internal_temp(temperature)); }

	bool set_msg (const uint8_t& line, const char* msg, const std::size_t& len) {
		std::lock_guard<std::mutex> lock (this->_m_msg);
		char* ptr = reinterpret_cast<char*>(this->_msg[line].load ());
		ptr = static_cast<char*>(heap_caps_realloc (ptr, len + 1, MALLOC_CAP_8BIT));
		if (ptr != nullptr) {
			for (std::size_t i = 0; i < len; ++i) {
				ptr[i] = msg[i];
			}
			ptr[len] = '\0';
			this->_msg[line] = reinterpret_cast<std::uintptr_t>(ptr);
			return true;
		} else {
			this->_msg[line] = 0;
		}
		return false;
	}

	inline const char* get_msg (const uint8_t& line) const {
		std::lock_guard<std::mutex> lock (this->_m_msg);
		return reinterpret_cast<const char*>(this->_msg[line].load ());
	}
	/*
	 *	bool generate_msg(const char *buf, const uint8_t buf_len, const uint8_t line) const {
	 *		std::lock_guard<std::mutex> lock (this->_m_msg);
	 *		std::size_t i_buf = 0;
	 *		std::size_t i_msg = 0;
	 *		bool parse = false;
	 *		char* msg = static_cast<const char*>(this->_msg[line]);
	 *		while (i_buf < buf_len && msg[i_msg] != '\0') {
	 *			if (parse) {
	 *
} else if (msg[i_msg] == '%') {
	if (msg[i_msg + 1] == '%') {
		buf[i_buf] = '%';
		++i_buf;
		i_msg += 2;
} else {
	parse = true;
	++i_msg;
}
} else {
	buf[i_buf] = msg[i_msg];
	++i_buf;
	++i_msg;
}
}
}
*/
	private:
		static inline constexpr float internal_temp_to_float (const uint16_t& t) noexcept { return ((static_cast<float>(t)) / 10); }
		static inline constexpr int_temp_t float_to_internal_temp (const float& t) noexcept { return (t * 10); }
};

#endif /* STATUS_H */
