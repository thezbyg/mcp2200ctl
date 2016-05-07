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
#ifndef HEADER_SET_EEPROM_COMMAND_H_
#define HEADER_SET_EEPROM_COMMAND_H_
#include "command.h"
#include "target.h"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
namespace command_line
{
	struct SetEepromCommand: public Command
	{
		SetEepromCommand();
		virtual ~SetEepromCommand();
		virtual void addOptions(boost::program_options::options_description &options, boost::program_options::options_description &hidden_options);
		virtual void addPositionalOptions(boost::program_options::positional_options_description &positional_options);
		virtual bool checkOptions(boost::program_options::variables_map &variable_map);
		virtual bool run();
		private:
		Target m_target;
		int m_address;
		HexOption<uint8_t> m_value;
	};
}
#endif /* HEADER_SET_EEPROM_COMMAND_H_ */
