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
#include "target.h"
#include "mcp2200.h"
#include <boost/program_options/option.hpp>
#include <iostream>
namespace po = boost::program_options;
using namespace std;

namespace command_line
{
	ostream &operator<<(ostream &stream, const VendorProduct &vendor_product)
	{
		ostream_state_saver state(stream);
		stream << "vendor ID:" << setfill('0') << hex << setw(4) << vendor_product.getVendorId() << ", product ID:" << setw(4) << vendor_product.getProductId();
		return stream;
	}
	Target::Target():
		m_serial_set(false),
		m_path_set(false)
	{
	}
	void Target::addOptions(po::options_description &options, po::options_description &hidden_options)
	{
		VendorProduct::addOptions(options, hidden_options);
		options.add_options()
			("serial,S", po::value<string>(&m_serial), "device serial number")
			("path,D", po::value<string>(&m_path), "device path")
			;
	}
	bool Target::checkOptions(po::variables_map &variable_map)
	{
		VendorProduct::checkOptions(variable_map);
		m_serial_set = variable_map.count("serial") > 0;
		m_path_set = variable_map.count("path") > 0;
		return true;
	}
	bool Target::isPathSet() const
	{
		return m_path_set;
	}
	bool Target::isSerialSet() const
	{
		return m_serial_set;
	}
	const std::string &Target::getPath() const
	{
		return m_path;
	}
	const std::string &Target::getSerial() const
	{
		return m_serial;
	}
	bool Target::open(mcp2200::Device &device)
	{
		if (isPathSet()){
			if (device.open(getPath())){
				return true;
			}else{
				cerr << "could not open device (" << getPath() << ")\n";
				return false;
			}
		}
		if (isSerialSet()){
			if (device.open(getVendorId(), getProductId(), getSerial().c_str())){
				return true;
			}else{
				cerr << "could not open device (" << *this << ", serial:" << getSerial() << ")\n";
				return false;
			}
		}
		if (device.open(getVendorId(), getProductId())){
			return true;
		}else{
			cerr << "could not open device (" << *this << ")\n";
			return false;
		}
	}
}
