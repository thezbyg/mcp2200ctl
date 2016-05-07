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
#include "vendor_product.h"
namespace po = boost::program_options;
namespace command_line
{
	void VendorProduct::addOptions(boost::program_options::options_description &options, boost::program_options::options_description &hidden_options)
	{
		options.add_options()
			("vendor,V", po::value<HexOption<uint16_t>>(&m_vendor_id)->default_value(HexOption<uint16_t>(0x04d8), "04d8"), "device vendor ID")
			("product,P", po::value<HexOption<uint16_t>>(&m_product_id)->default_value(HexOption<uint16_t>(0x00df), "00df"), "device product ID")
			;
	}
	bool VendorProduct::checkOptions(po::variables_map &variable_map)
	{
		return true;
	}
	const uint16_t& VendorProduct::getVendorId() const
	{
		return m_vendor_id;
	}
	const uint16_t& VendorProduct::getProductId() const
	{
		return m_product_id;
	}
}
