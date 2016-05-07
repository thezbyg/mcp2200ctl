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
#include "get_command.h"
#include "mcp2200.h"
#include "helpers.h"
#include "format.h"
#include <iostream>
using namespace std;
namespace po = boost::program_options;
namespace command_line
{
	GetCommand::GetCommand():
		Command("get", "get GPIO state")
	{
	}
	GetCommand::~GetCommand()
	{
	}
	void GetCommand::addOptions(po::options_description &options, po::options_description &hidden_options)
	{
		m_target.addOptions(options, hidden_options);
	}
	bool GetCommand::checkOptions(po::variables_map &variable_map)
	{
		return m_target.checkOptions(variable_map);
	}
	bool GetCommand::run()
	{
		mcp2200::Device device;
		if (!m_target.open(device)){
			return false;
		}
		mcp2200::Command response;
		if (device.readAll(response)){
			cout << "GPIO values: " << BitMap<uint8_t>(response.getGpioValues()) << "\n";
		}
		device.close();
		return true;
	}
}
