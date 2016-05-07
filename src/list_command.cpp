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
#include "list_command.h"
#include "mcp2200.h"
#include <iostream>
namespace po = boost::program_options;
using namespace std;
namespace command_line
{
	ListCommand::ListCommand():
		Command("list", "list devices")
	{
	}
	ListCommand::~ListCommand()
	{
	}
	void ListCommand::addOptions(po::options_description &options, po::options_description &hidden_options)
	{
		m_vendor_product.addOptions(options, hidden_options);
	}
	bool ListCommand::checkOptions(po::variables_map &variable_map)
	{
		return m_vendor_product.checkOptions(variable_map);
	}
	bool ListCommand::run()
	{
		mcp2200::Device d;
		d.find(m_vendor_product.getVendorId(), m_vendor_product.getProductId());
		for (size_t i = 0; i < d.getCount(); i++){
			cout
				<< setw(3) << i + 1 << ","
				<< " \"" << d[i].manufacturer << "\","
				<< " \"" << d[i].product << "\","
				<< " \"" << d[i].serial << "\","
				<< " \"" << d[i].path << "\""
				<< "\n";
		}
		return d.getCount() > 0;
	}
}
