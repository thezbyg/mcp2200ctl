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
#ifndef HEADER_CONFIGURE_COMMAND_H_
#define HEADER_CONFIGURE_COMMAND_H_
#include "command.h"
#include "target.h"
#include "helpers.h"
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
namespace command_line
{
	struct ConfigureCommand: public Command
	{
		ConfigureCommand();
		virtual ~ConfigureCommand();
		virtual void addOptions(boost::program_options::options_description &options, boost::program_options::options_description &hidden_options);
		virtual bool checkOptions(boost::program_options::variables_map &variable_map);
		virtual bool run();
		private:
		Target m_target;
		BitMap<uint8_t> m_direction, m_default;
		mcp2200::LedMode m_rx_led, m_tx_led;
		bool m_invert, m_suspend, m_configuration, m_flow_control, m_blink_speed;
		bool m_direction_set, m_default_set, m_rx_led_set, m_tx_led_set, m_invert_set, m_suspend_set, m_configuration_set, m_flow_control_set, m_blink_speed_set, m_print;
	};
}
#endif /* HEADER_CONFIGURE_COMMAND_H_ */
