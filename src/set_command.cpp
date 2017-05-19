/*
Copyright (c) 2016-2017, Albertas Vyšniauskas
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
#include "set_command.h"
#include "mcp2200.h"
#include "helpers.h"
#include <iostream>
using namespace std;
namespace po = boost::program_options;
namespace command_line
{
	SetCommand::SetCommand():
		Command("set", "set GPIO state", "VALUES")
	{
	}
	SetCommand::~SetCommand()
	{
	}
	void SetCommand::addOptions(po::options_description &options, po::options_description &hidden_options)
	{
		m_target.addOptions(options, hidden_options);
		options.add_options()
			("pin", po::value<int>(&m_pin), "pin index (starting from 0)")
			("value", po::value<bool>(&m_value), "pin value")
			("mask", po::value<BitMap<uint8_t>>(&m_mask), "pin mask")
		;
		hidden_options.add_options()
			("values", po::value<BitMap<uint8_t>>(&m_values), "")
		;
	}
	void SetCommand::addPositionalOptions(boost::program_options::positional_options_description &positional_options)
	{
		positional_options.add("values", 1);
	}
	bool SetCommand::checkOptions(po::variables_map &variable_map)
	{
		if (!m_target.checkOptions(variable_map)) return false;
		m_all_values = variable_map.count("values") > 0;
		m_one_pin = (variable_map.count("pin") > 0 && variable_map.count("value") > 0);
		m_has_mask = variable_map.count("mask") > 0;
		if (!m_all_values && !m_one_pin){
			cerr << "VALUES or pin/value pair must be defined\n";
			return false;
		}
		if (m_one_pin){
			if (m_pin < 0 || m_pin > 7){
				cerr << "pin index is out of range [0:7]\n";
				return false;
			}
		}
		return true;
	}
	bool SetCommand::run()
	{
		mcp2200::Device device;
		if (!m_target.open(device)){
			return false;
		}
		if (!device.writeAfterRead([&](mcp2200::Command &command){
			command.setCommand(mcp2200::CommandType::set_clear_outputs);
			uint8_t gpio_mask = command.getIoMask();
			if (m_has_mask)
				gpio_mask &= m_mask;
			if (m_all_values){
				command.setGpioValues(m_values & gpio_mask, ~m_values & gpio_mask);
			}else{
				uint8_t gpio = (m_value ? 1 : 0) << m_pin;
				gpio_mask &= 1 << m_pin;
				command.setGpioValues(gpio & gpio_mask, ~gpio & gpio_mask);
			}
			return true;
		})){
			device.close();
			return false;
		}
		device.close();
		return false;
	}
}
