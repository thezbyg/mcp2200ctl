/*
Copyright (c) 2016, Albertas Vyšniauskas
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "mcp2200.h"
#include <hidapi/hidapi.h>
#include <codecvt>
#include <locale>
#include <iostream>
#include <boost/endian/conversion.hpp>
using namespace std;
namespace mcp2200
{
	inline void setBit(uint8_t &x, int bit, bool value)
	{
		if (value)
			x |= 1 << bit;
		else
			x &= ~(1 << bit);
	}
	inline bool getBit(const uint8_t x, int bit)
	{
		return ((x >> bit) & 1) == 1;
	}
	template <typename T>
	void fromUtf8(const string& source, basic_string<T, char_traits<T>, allocator<T>>& result)
	{
		wstring_convert<codecvt_utf8_utf16<T>, T> convertor;
		result = convertor.from_bytes(source);
	}
	template <typename T>
	void toUtf8(const basic_string<T, char_traits<T>, allocator<T>>& source, string &result)
	{
		wstring_convert<codecvt_utf8_utf16<T>, T> convertor;
		result = convertor.to_bytes(source);
	}
	const static int hid_report_size = 16;
	Command::Command()
	{
		memset(this, 0, sizeof(Command));
	}
	Command::Command(const Command &command)
	{
		memcpy(this, &command, sizeof(Command));
	}
	Command &Command::setCommand(CommandType command_type)
	{
		this->command_type = static_cast<uint8_t>(command_type);
		return *this;
	}
	int Command::length() const
	{
		return hid_report_size;
	}
	const uint8_t *Command::getPointer() const
	{
		return reinterpret_cast<const uint8_t *>(this);
	}
	uint8_t *Command::getPointer()
	{
		return reinterpret_cast<uint8_t *>(this);
	}
	Command &Command::setBaudRate(int baud_rate)
	{
		using namespace boost::endian;
		int divisor = 12000000 / baud_rate - 1;
		configure.baud_rate = native_to_big(divisor);
		return *this;
	}
	int Command::getBaudRate() const
	{
		using namespace boost::endian;
		return 12000000 / (big_to_native(read_all_response.baud_rate) + 1);
	}
	Command &Command::setIoDirections(uint8_t io_directions)
	{
		auto mask = getIoMask();
		configure.io_directions = (configure.io_directions & ~mask) | (io_directions & mask);
		return *this;
	}
	Command &Command::setDefaultValues(uint8_t default_values)
	{
		auto mask = getIoMask();
		configure.default_values = (configure.default_values & ~mask) | (default_values & mask);
		return *this;
	}
	void Command::configureRxLed(bool enable, bool toggle)
	{
		setBit(configure.alt_pins, 3, enable);
		setBit(configure.alt_pin_options, 7, toggle);
	}
	void Command::configureTxLed(bool enable, bool toggle)
	{
		setBit(configure.alt_pins, 2, enable);
		setBit(configure.alt_pin_options, 6, toggle);
	}
	LedMode Command::getRxLedMode() const
	{
		if (getBit(read_all_response.alt_pins, 2)){
			if (getBit(read_all_response.alt_pin_options, 6)){
				return LedMode::toggle;
			}else{
				return LedMode::blink;
			}
		}else{
			return getBit(read_all_response.default_values, 6) ? LedMode::on : LedMode::off;
		}
	}
	void Command::setBlinkSpeed(bool slow)
	{
		setBit(configure.alt_pin_options, 5, slow);
	}
	bool Command::getBlinkSpeed() const
	{
		return getBit(read_all_response.alt_pin_options, 5);
	}
	Command &Command::setRxLedMode(LedMode led_mode)
	{
		switch (led_mode){
			case LedMode::off:
				configureRxLed(false, false);
				setBit(configure.io_directions, 6, false);
				setBit(configure.default_values, 6, false);
				break;
			case LedMode::on:
				configureRxLed(false, false);
				setBit(configure.io_directions, 6, false);
				setBit(configure.default_values, 6, true);
				break;
			case LedMode::blink:
				configureRxLed(true, false);
				break;
			case LedMode::toggle:
				configureRxLed(true, true);
				break;
		}
		return *this;
	}
	LedMode Command::getTxLedMode() const
	{
		if (getBit(read_all_response.alt_pins, 3)){
			if (getBit(read_all_response.alt_pin_options, 6)){
				return LedMode::toggle;
			}else{
				return LedMode::blink;
			}
		}else{
			return getBit(read_all_response.default_values, 7) ? LedMode::on : LedMode::off;
		}
	}
	Command &Command::setTxLedMode(LedMode led_mode)
	{
		switch (led_mode){
			case LedMode::off:
				configureTxLed(false, false);
				setBit(configure.io_directions, 7, false);
				setBit(configure.default_values, 7, false);
				break;
			case LedMode::on:
				configureTxLed(false, false);
				setBit(configure.io_directions, 7, false);
				setBit(configure.default_values, 7, true);
				break;
			case LedMode::blink:
				configureTxLed(true, false);
				break;
			case LedMode::toggle:
				configureTxLed(true, true);
				break;
		}
		return *this;
	}
	Command &Command::setSuspend(bool suspend)
	{
		setBit(configure.alt_pins, 7, suspend);
		return *this;
	}
	Command &Command::setUsbConfigure(bool usb_configure)
	{
		setBit(configure.alt_pins, 6, usb_configure);
		return *this;
	}
	Command &Command::setFlowControl(bool flow_control)
	{
		setBit(configure.alt_pin_options, 0, flow_control);
		return *this;
	}
	Command &Command::setInvert(bool invert)
	{
		setBit(configure.alt_pin_options, 1, invert);
		return *this;
	}
	bool Command::getInvert() const
	{
		return getBit(read_all_response.alt_pin_options, 1);
	}
	bool Command::getFlowControl() const
	{
		return getBit(read_all_response.alt_pin_options, 0);
	}
	bool Command::getSuspend() const
	{
		return getBit(read_all_response.alt_pins, 7);
	}
	bool Command::getUsbConfigure() const
	{
		return getBit(read_all_response.alt_pins, 6);
	}
	uint8_t Command::getGpioValues() const
	{
		return read_all_response.gpio_values;
	}
	uint8_t Command::getIoDirections() const
	{
		return read_all_response.io_directions;
	}
	uint8_t Command::getDefaultValues() const
	{
		return read_all_response.default_values;
	}
	Command &Command::setGpioValues(uint8_t set, uint8_t clear)
	{
		set_clear_outputs.set = set;
		set_clear_outputs.clear = clear;
		return *this;
	}
	Command &Command::setEepromAddress(uint8_t address)
	{
		read_eeprom.address = address;
		return *this;
	}
	uint8_t Command::getEepromAddress() const
	{
		return read_eeprom_response.address;
	}
	uint8_t Command::getEepromValue() const
	{
		return read_eeprom_response.value;
	}
	Command &Command::setEepromValue(uint8_t value)
	{
		write_eeprom.value = value;
		return *this;
	}
	static bool isAlternativeSet(LedMode led_mode)
	{
		switch (led_mode){
			case LedMode::off:
			case LedMode::on:
				return false;
			case LedMode::blink:
			case LedMode::toggle:
				return true;
		}
		return false;
	}
	uint8_t Command::getIoMask() const
	{
		return ~((getSuspend() ? 1 << 0 : 0) | (getUsbConfigure() ? 1 << 1 : 0) | (isAlternativeSet(getRxLedMode()) ? 1 << 6 : 0) | (isAlternativeSet(getTxLedMode()) ? 1 << 7 : 0));
	}
	DeviceInformation::DeviceInformation()
	{
	}
	DeviceInformation::DeviceInformation(const char *path, const char *serial, const char *manufacturer, const char *product, uint16_t release_number)
	{
		if (path)
			this->path = path;
		if (serial)
			this->serial = serial;
		if (manufacturer)
			this->manufacturer = manufacturer;
		if (product)
			this->product = product;
		if (release_number)
			this->release_number = release_number;
	}
	bool Device::getManufacturer(string &manufacturer)
	{
		if (!m_handle) return false;
		wstring manufacturer_wide;
		manufacturer_wide.resize(255);
		hid_get_manufacturer_string(m_handle, const_cast<wchar_t*>(manufacturer_wide.c_str()), manufacturer_wide.length());
		toUtf8(manufacturer_wide, manufacturer);
		return true;
	}
	bool Device::getProduct(string &product)
	{
		if (!m_handle) return false;
		wstring product_wide;
		product_wide.resize(255);
		hid_get_product_string(m_handle, const_cast<wchar_t*>(product_wide.c_str()), product_wide.length());
		toUtf8(product_wide, product);
		return true;
	}
	bool Device::getSerial(string &serial)
	{
		if (!m_handle) return false;
		wstring serial_wide;
		serial_wide.resize(255);
		hid_get_serial_number_string(m_handle, const_cast<wchar_t*>(serial_wide.c_str()), serial_wide.length());
		toUtf8(serial_wide, serial);
		return true;
	}
	Device::Device():
		m_handle(nullptr)
	{
	}
	Device::~Device()
	{
		close();
	}
	bool Device::configure(std::function<void(Command &command)> actions)
	{
		Command command = {};
		if (!readAll(command)) return false;
		command
			.setCommand(CommandType::configure)
			;
		actions(command);
		return write(command);
	}
	bool Device::configure(uint8_t io_directions, int baud_rate, LedMode rx_led_mode, LedMode tx_led_mode, bool flow_control, bool usb_configure, bool suspend, bool invert)
	{
		return configure([io_directions, baud_rate, rx_led_mode, tx_led_mode, flow_control, usb_configure, suspend, invert](Command &command){
		command
			.setIoDirections(io_directions)
			.setBaudRate(baud_rate)
			.setRxLedMode(rx_led_mode)
			.setTxLedMode(tx_led_mode)
			.setFlowControl(flow_control)
			.setUsbConfigure(usb_configure)
			.setSuspend(suspend)
			.setInvert(invert)
			;
		});
	}
	bool Device::find(uint16_t vendor_id, uint16_t product_id)
	{
		hid_device_info *devices = hid_enumerate(vendor_id, product_id);
		if (!devices) return false;
		size_t device_count = 0;
		hid_device_info *i = devices;
		while (i != nullptr){
			device_count++;
			i = i->next;
		}
		m_found.resize(device_count);
		size_t device_i = 0;
		i = devices;
		while (i != nullptr){
			string manufacturer_n, product_n, serial_n;
			if (i->manufacturer_string != nullptr)
				toUtf8(wstring(i->manufacturer_string), manufacturer_n);
			if (i->product_string != nullptr)
				toUtf8(wstring(i->product_string), product_n);
			if (i->serial_number != nullptr)
				toUtf8(wstring(i->serial_number), serial_n);
			m_found[device_i++] = DeviceInformation(i->path, serial_n.c_str(), manufacturer_n.c_str(), product_n.c_str(), i->release_number);
			i = i->next;
		}
		hid_free_enumeration(devices);
		return true;
	}
	size_t Device::getCount() const
	{
		return m_found.size();
	}
	const DeviceInformation &Device::get(size_t index) const
	{
		return m_found[index];
	}
	const DeviceInformation &Device::operator[](size_t index) const
	{
		return m_found[index];
	}
	bool Device::open(uint16_t vendor_id, uint16_t product_id, const char *serial)
	{
		close();
		if (serial != nullptr){
			wstring serial_w;
			fromUtf8(serial, serial_w);
			m_handle = hid_open(vendor_id, product_id, serial_w.c_str());
		}else{
			m_handle = hid_open(vendor_id, product_id, nullptr);
		}
		if (!m_handle) return false;
		return true;
	}
	bool Device::open(const DeviceInformation &device)
	{
		close();
		m_handle = hid_open_path(device.path.c_str());
		if (!m_handle) return false;
		return true;
	}
	bool Device::open(const char *device_path)
	{
		close();
		m_handle = hid_open_path(device_path);
		if (!m_handle) return false;
		return true;
	}
	bool Device::open(const std::string &device_path)
	{
		return open(device_path.c_str());
	}
	void Device::close()
	{
		if (m_handle != nullptr)
			hid_close(m_handle);
		m_handle = nullptr;
	}
	bool Device::isOpen()
	{
		return m_handle != nullptr;
	}
	bool Device::write(const Command &command)
	{
		if (hid_write(m_handle, command.getPointer(), command.length()) < 0) return false;
		return true;
	}
	bool Device::read(Command &response)
	{
		if (hid_read(m_handle, response.getPointer(), response.length()) < 0) return false;
		return true;
	}
	bool Device::readAll(Command &response)
	{
		Command command = {};
		command
			.setCommand(CommandType::read_all)
			;
		return write(command) && read(response);
	}
	bool Device::setGpioValues(uint8_t values)
	{
		Command command = {};
		command
			.setCommand(CommandType::set_clear_outputs)
			.setGpioValues(values, ~values)
			;
		return write(command);
	}
	bool Device::setInvert(bool invert)
	{
		return configure([invert](Command &command){
			command.setInvert(invert);
		});
	}
	bool Device::setSuspend(bool suspend)
	{
		return configure([suspend](Command &command){
			command.setSuspend(suspend);
		});
	}
	bool Device::setIoDirections(uint8_t io_directions)
	{
		return configure([io_directions](Command &command){
			command.setIoDirections(io_directions);
		});
	}
	bool Device::setDefaultValues(uint8_t default_values)
	{
		return configure([default_values](Command &command){
			command.setDefaultValues(default_values);
		});
	}
	bool Device::readEeprom(uint8_t address, uint8_t &value)
	{
		Command command = {};
		command
			.setCommand(CommandType::read_eeprom)
			.setEepromAddress(address)
			;
		Command response = {};
		if (!(write(command) && read(response))) return false;
		value = response.getEepromValue();
		return true;
	}
	bool Device::writeEeprom(uint8_t address, uint8_t value)
	{
		Command command = {};
		command
			.setCommand(CommandType::write_eeprom)
			.setEepromAddress(address)
			.setEepromValue(value)
			;
		return write(command);
	}
	bool Device::setString(ConfigurationType type, const char *value)
	{
		using namespace boost::endian;
		Command command = {};
		command.setCommand(CommandType::base_configure);
		wstring value_w;
		fromUtf8(value, value_w);
		if (value_w.length() > 63){
			return false;
		}
		int length = value_w.length() * 2 + 2;
		command.base_configure.configuration_type = static_cast<uint8_t>(type);
		size_t position = 0;
		for (uint8_t i = 0; i < 16; i++){
			command.base_configure.set_string.index = i;
			if (i == 0){
				command.base_configure.set_string.chars[0] = native_to_big(static_cast<uint16_t>(0x03 | (length << 8)));
				for (int j = 0; j < 3; j++){
					command.base_configure.set_string.chars[j + 1] = position < value_w.length() ? native_to_big(static_cast<uint16_t>(value_w[position])) : 0;
					position++;
				}
			}else{
				for (int j = 0; j < 4; j++){
					command.base_configure.set_string.chars[j] = position < value_w.length() ? native_to_big(static_cast<uint16_t>(value_w[position])) : 0;
					position++;
				}
			}
			if (!write(command)){
				return false;
			}
		}
		return true;
	}
	bool Device::setManufacturer(const char *value)
	{
		return setString(ConfigurationType::set_manufacturer, value);
	}
	bool Device::setProduct(const char *value)
	{
		return setString(ConfigurationType::set_product, value);
	}
	bool Device::setVendorProductIds(uint16_t vendor_id, uint16_t product_id)
	{
		using namespace boost::endian;
		Command command = {};
		command.setCommand(CommandType::base_configure);
		command.base_configure.configuration_type = static_cast<uint8_t>(ConfigurationType::set_vid_pid);
		command.base_configure.set_vid_pid.vendor_id = native_to_big(vendor_id);
		command.base_configure.set_vid_pid.product_id = native_to_big(product_id);
		return write(command);
	}
}
