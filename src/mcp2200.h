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
#ifndef HEADER_MCP2200_H_
#define HEADER_MCP2200_H_
#include <stdint.h>
#include <string>
#include <vector>
#include <functional>
typedef struct hid_device_ hid_device;
namespace mcp2200
{
	enum class LedMode:uint8_t
	{
		off,
		on,
		blink,
		toggle,
	};
	enum class CommandType: uint8_t
	{
		base_configure = 0x01,
		set_clear_outputs = 0x08,
		configure = 0x10,
		read_eeprom = 0x20,
		write_eeprom = 0x40,
		read_all = 0x80,
	};
	enum class ConfigurationType:uint8_t
	{
		set_vid_pid = 0x00,
		set_manufacturer = 0x01,
		set_product = 0x02,
	};
#pragma pack(push,1)
	struct Command
	{
		uint8_t command_type;
		union
		{
			struct
			{
				uint8_t reserved1[10];
				uint8_t set;
				uint8_t clear;
				uint8_t reserved2[3];
			}set_clear_outputs;
			struct
			{
				uint8_t reserved1[3];
				uint8_t io_directions;
				uint8_t alt_pins;
				uint8_t default_values;
				uint8_t alt_pin_options;
				uint16_t baud_rate;
				uint8_t reserved2[6];
			}configure;
			struct
			{
				uint8_t eeprom_address;
				uint8_t reserved1;
				uint8_t eeprom_value;
				uint8_t io_directions;
				uint8_t alt_pins;
				uint8_t default_values;
				uint8_t alt_pin_options;
				uint16_t baud_rate;
				uint8_t gpio_values;
				uint8_t reserved2[5];
			}read_all_response;
			struct
			{
				uint8_t address;
				uint8_t reserved2[14];
			}read_eeprom;
			struct
			{
				uint8_t address;
				uint8_t reserved1;
				uint8_t value;
				uint8_t reserved2[12];
			}read_eeprom_response;
			struct
			{
				uint8_t address;
				uint8_t value;
				uint8_t reserved2[13];
			}write_eeprom;
			struct
			{
				uint8_t configuration_type;
				union
				{
					struct{
						uint16_t vendor_id;
						uint16_t product_id;
						uint8_t reserved1[12];
					}set_vid_pid;
					struct{
						uint8_t index;
						uint16_t chars[4];
						uint8_t reserved1[5];
					}set_string;
				};
			}base_configure;
		};
		Command();
		Command(const Command &command);
		Command &setCommand(CommandType command_type);
		int length() const;
		const uint8_t *getPointer() const;
		uint8_t *getPointer();
		Command &setBaudRate(int baud_rate);
		int getBaudRate() const;
		Command &setIoDirections(uint8_t io_directions);
		Command &setDefaultValues(uint8_t default_values);
		void configureRxLed(bool enable, bool toggle);
		void configureTxLed(bool enable, bool toggle);
		LedMode getRxLedMode() const;
		Command &setRxLedMode(LedMode led_mode);
		LedMode getTxLedMode() const;
		Command &setTxLedMode(LedMode led_mode);
		Command &setSuspend(bool suspend);
		Command &setUsbConfigure(bool usb_configure);
		Command &setFlowControl(bool flow_control);
		Command &setInvert(bool invert);
		Command &setBlinkSpeed(bool slow);
		bool getInvert() const;
		bool getFlowControl() const;
		bool getSuspend() const;
		bool getUsbConfigure() const;
		bool getBlinkSpeed() const;
		uint8_t getGpioValues() const;
		Command &setGpioValues(uint8_t set, uint8_t clear);
		Command &setEepromAddress(uint8_t address);
		uint8_t getEepromAddress() const;
		uint8_t getEepromValue() const;
		Command &setEepromValue(uint8_t value);
		uint8_t getIoDirections() const;
		uint8_t getDefaultValues() const;
		uint8_t getIoMask() const;
	};
#pragma pack(pop)
	struct DeviceInformation
	{
		DeviceInformation();
		DeviceInformation(const char *path, const char *serial, const char *manufacturer, const char *product, uint16_t release_number);
		std::string path;
		std::string serial;
		std::string manufacturer;
		std::string product;
		uint16_t release_number;
	};
	struct Device
	{
		const static uint16_t defaultVendorId = 0x04d8;
		const static uint16_t defaultProductId = 0x00df;
		Device();
		~Device();
		bool find(uint16_t vendor_id = 0x04d8, uint16_t product_id = 0x00df);
		size_t getCount() const;
		const DeviceInformation &get(size_t index) const;
		const DeviceInformation &operator[](size_t index) const;
		bool open(uint16_t vendor_id = 0x04d8, uint16_t product_id = 0x00df, const char *serial = nullptr);
		bool open(const DeviceInformation &device);
		bool open(const char *device_path);
		bool open(const std::string &device_path);
		void close();
		bool isOpen();
		bool write(const Command &command);
		bool read(Command &command);
		bool readAll(Command &response);
		bool configure(uint8_t io_directions, int baud_rate, LedMode rx_led_mode, LedMode tx_led_mode, bool flow_control, bool usb_configure, bool suspend, bool invert);
		bool configure(std::function<void(Command &)> actions);
		bool setGpioValues(uint8_t values);
		bool setInvert(bool invert);
		bool setSuspend(bool suspend);
		bool setIoDirections(uint8_t io_directions);
		bool setDefaultValues(uint8_t default_values);
		bool readEeprom(uint8_t address, uint8_t &value);
		bool writeEeprom(uint8_t address, uint8_t value);
		bool getManufacturer(std::string &manufacturer);
		bool getProduct(std::string &product);
		bool getSerial(std::string &serial);
		bool setManufacturer(const char *value);
		bool setProduct(const char *value);
		bool setVendorProductIds(uint16_t vendor_id, uint16_t product_id);
		bool setString(ConfigurationType type, const char *value);
		void setReadTimeout(int timeout);
		private:
		hid_device *m_handle;
		std::vector<DeviceInformation> m_found;
		int m_timeout;
	};
};
#endif /* HEADER_MCP2200_H_ */
