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
#include "describe_command.h"
#include "mcp2200.h"
#include <iostream>
using namespace std;
namespace po = boost::program_options;
namespace command_line
{
	DescribeCommand::DescribeCommand():
		Command("describe", "get or set device description")
	{
	}
	DescribeCommand::~DescribeCommand()
	{
	}
	void DescribeCommand::addOptions(po::options_description &options, po::options_description &hidden_options)
	{
		m_target.addOptions(options, hidden_options);
		options.add_options()
			("set-manufacturer,m", po::value<string>(&m_manufacturer), "set manufacturer string")
			("set-product,p", po::value<string>(&m_product), "set product string")
			("set-vendor-id", po::value<HexOption<uint16_t>>(&m_vendor_id), "set vendor ID")
			("set-product-id", po::value<HexOption<uint16_t>>(&m_product_id), "set product ID")
		;
	}
	bool DescribeCommand::checkOptions(po::variables_map &variable_map)
	{
		if (!m_target.checkOptions(variable_map)) return false;
		m_manufacturer_set = variable_map.count("set-manufacturer") > 0;
		m_product_set = variable_map.count("set-product") > 0;
		m_vendor_id_set = variable_map.count("set-vendor-id") > 0;
		m_product_id_set = variable_map.count("set-product-id") > 0;
		m_print = !(m_manufacturer_set || m_product_set || m_vendor_id_set || m_product_id_set);
		return true;
	}
	bool DescribeCommand::run()
	{
		mcp2200::Device device;
		if (!m_target.open(device)){
			return false;
		}
		if (!m_print){
			if (m_manufacturer_set)
				device.setManufacturer(m_manufacturer.c_str());
			if (m_product_set)
				device.setProduct(m_product.c_str());
			if (m_vendor_id_set || m_product_id_set){
				auto vendor_id = m_vendor_id_set ? static_cast<uint16_t>(m_vendor_id) : m_target.getVendorId();
				auto product_id = m_product_id_set ? static_cast<uint16_t>(m_product_id) : m_target.getProductId();
				device.setVendorProductIds(vendor_id, product_id);
			}
		}else{
			string manufacturer, product, serial;
			device.getManufacturer(manufacturer);
			device.getProduct(product);
			device.getSerial(serial);
			cout
				<< "Manufacturer: " << manufacturer << "\n"
				<< "Product: " << product << "\n"
				<< "Serial: " << serial << "\n";
		}
		device.close();
		return true;
	}
}
