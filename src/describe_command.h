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
#ifndef HEADER_DESCRIBE_COMMAND_H_
#define HEADER_DESCRIBE_COMMAND_H_
#include "command.h"
#include "target.h"
#include "helpers.h"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
namespace command_line
{
	struct DescribeCommand: public Command
	{
		DescribeCommand();
		virtual ~DescribeCommand();
		virtual void addOptions(boost::program_options::options_description &options, boost::program_options::options_description &hidden_options);
		virtual bool checkOptions(boost::program_options::variables_map &variable_map);
		virtual bool run();
		private:
		Target m_target;
		std::string m_manufacturer, m_product;
		HexOption<uint16_t> m_vendor_id, m_product_id;
		bool m_manufacturer_set, m_product_set, m_vendor_id_set, m_product_id_set, m_print;
	};
}
#endif /* HEADER_DESCRIBE_COMMAND_H_ */
