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
#include "format.h"
#include "helpers.h"
#include <iostream>
#include <boost/program_options/options_description.hpp>
using namespace std;
namespace command_line
{
	ostream& operator<<(ostream& stream, const mcp2200::LedMode &led_mode)
	{
		using namespace mcp2200;
		switch (led_mode){
			case LedMode::off:
				stream << "off";
				break;
			case LedMode::on:
				stream << "on";
				break;
			case LedMode::blink:
				stream << "blink";
				break;
			case LedMode::toggle:
				stream << "toggle";
				break;
		}
		return stream;
	};
	ostream& operator<<(ostream& stream, const mcp2200::Command &response)
	{
		stream << "Default baud rate: " << response.getBaudRate() << "\n"
			<< "GPIO directions: " << BitMap<uint8_t>(response.getIoDirections(), 'o', 'i') << "\n"
			<< "GPIO default values: " << BitMap<uint8_t>(response.getDefaultValues()) << "\n"
			<< "GPIO values: " << BitMap<uint8_t>(response.getGpioValues()) << "\n"
			<< "RX LED (GPIO pin 6): " << response.getRxLedMode() << "\n"
			<< "TX LED (GPIO pin 7): " << response.getTxLedMode() << "\n"
			<< "Blink speed: " << (response.getBlinkSpeed() ? "slow" : "fast") << "\n"
			<< "Invert RX/TX/RTS/CTS: " << Boolean(response.getInvert()) << "\n"
			<< "USB suspend pin (GPIO pin 0): " << Boolean(response.getSuspend()) << "\n"
			<< "USB configuration pin (GPIO pin 1): " << Boolean(response.getUsbConfigure()) << "\n"
			<< "Hardware flow control RTS/CTS: " << Boolean(response.getFlowControl()) << "\n";
		return stream;
	};
}
