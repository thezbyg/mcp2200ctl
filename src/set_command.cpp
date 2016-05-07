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
			("values",  po::value<BitMap<uint8_t>>(&m_values), "")
		;
	}
	void SetCommand::addPositionalOptions(boost::program_options::positional_options_description &positional_options)
	{
		positional_options.add("values", 1);
	}
	bool SetCommand::checkOptions(po::variables_map &variable_map)
	{
		if (!m_target.checkOptions(variable_map)) return false;
		if (variable_map.count("values") <= 0){
			cerr << "VALUES must be defined\n";
			return false;
		}
		return true;
	}
	bool SetCommand::run()
	{
		mcp2200::Device device;
		if (!m_target.open(device)){
			return false;
		}
		if (device.setGpioValues(m_values)){
			device.close();
			return true;
		}
		device.close();
		return false;
	}
}
