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
#include "configure_command.h"
#include "mcp2200.h"
#include "helpers.h"
#include "format.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
using namespace std;
namespace po = boost::program_options;
namespace mcp2200
{
	void validate(boost::any &v, const std::vector<std::string> &values, LedMode *target, int)
	{
		using namespace boost::program_options;
		validators::check_first_occurrence(v);
		auto value_string = validators::get_single_string(values);
		boost::algorithm::to_lower(value_string);
		LedMode led_mode = LedMode::off;
		if (value_string == "off")
			led_mode = LedMode::off;
		else if (value_string == "on")
			led_mode = LedMode::on;
		else if (value_string == "blink")
			led_mode = LedMode::blink;
		else if (value_string == "toggle")
			led_mode = LedMode::toggle;
		else
			throw validation_error(validation_error::invalid_option_value);
		v = boost::any(led_mode);
	}
}
namespace command_line
{
	ConfigureCommand::ConfigureCommand():
		Command("configure", "get or set device configuration")
	{
	}
	ConfigureCommand::~ConfigureCommand()
	{
	}
	void ConfigureCommand::addOptions(po::options_description &options, po::options_description &hidden_options)
	{
		m_target.addOptions(options, hidden_options);
		options.add_options()
			("direction,d", po::value<BitMap<uint8_t>>(&m_direction), "GPIO directions (use i and o symbols)")
			("default,D", po::value<BitMap<uint8_t>>(&m_default), "default GPIO values")
			("rxled,r", po::value<mcp2200::LedMode>(&m_rx_led), "receive LED mode")
			("txled,t", po::value<mcp2200::LedMode>(&m_tx_led), "transmit LED mode")
			("blink,b", po::value<bool>(&m_blink_speed), "enable fast blink")
			("invert,i", po::value<bool>(&m_invert), "invert RX/TX/RTS/CTS pins")
			("suspend,s", po::value<bool>(&m_suspend), "enable USB suspend pin (GPIO pin 0)")
			("configuration,c", po::value<bool>(&m_configuration), "enable USB configuration pin (GPIO pin 1)")
			("flow,f", po::value<bool>(&m_flow_control), "hardware flow control RTS/CTS")
		;
	}
	bool ConfigureCommand::checkOptions(po::variables_map &variable_map)
	{
		if (!m_target.checkOptions(variable_map)) return false;
		m_direction_set = variable_map.count("direction") > 0;
		m_default_set = variable_map.count("default") > 0;
		m_rx_led_set = variable_map.count("rxled") > 0;
		m_tx_led_set = variable_map.count("txled") > 0;
		m_blink_speed_set = variable_map.count("blink") > 0;
		m_invert_set = variable_map.count("invert") > 0;
		m_suspend_set = variable_map.count("suspend") > 0;
		m_configuration_set = variable_map.count("configuration") > 0;
		m_flow_control_set = variable_map.count("flow") > 0;
		m_print = !(m_direction_set || m_default_set || m_rx_led_set || m_tx_led_set || m_invert_set || m_suspend_set || m_configuration_set || m_flow_control_set | m_blink_speed_set);
		return true;
	}
	bool ConfigureCommand::run()
	{
		mcp2200::Device device;
		if (!m_target.open(device)){
			return false;
		}
		mcp2200::Command response;
		if (device.readAll(response)){
			if (m_print){
				cout << response;
			}else{
				mcp2200::Command command(response);
				command.setCommand(mcp2200::CommandType::configure);
				if (m_rx_led_set)
					command.setRxLedMode(m_rx_led);
				if (m_tx_led_set)
					command.setTxLedMode(m_tx_led);
				if (m_suspend_set)
					command.setSuspend(m_suspend);
				if (m_configuration_set)
					command.setUsbConfigure(m_configuration);
				if (m_invert_set)
					command.setInvert(m_invert);
				if (m_flow_control_set)
					command.setFlowControl(m_flow_control);
				if (m_direction_set)
					command.setIoDirections(m_direction);
				if (m_default_set)
					command.setDefaultValues(m_default);
				if (m_blink_speed_set)
					command.setBlinkSpeed(!m_blink_speed);
				if (!device.write(command)){
					cerr << "could not write configuration\n";
					device.close();
					return false;
				}
			}
		}
		device.close();
		return true;
	}
}
