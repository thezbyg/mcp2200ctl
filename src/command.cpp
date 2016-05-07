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
#include "command.h"
using namespace std;
namespace po = boost::program_options;
namespace command_line
{
	Command::Command(const char *name, const char *description)
	{
		m_name = name;
		m_description = description;
	}
	Command::Command(const char *name, const char *description, const char *command_pattern)
	{
		m_name = name;
		m_description = description;
		m_command_pattern = command_pattern;
	}
	Command::~Command()
	{
	}
	const char *Command::getName() const
	{
		return m_name.c_str();
	}
	const char *Command::getDescription() const
	{
		return m_description.c_str();
	}
	const char *Command::getCommandPattern() const
	{
		return m_command_pattern.c_str();
	}
	void Command::addOptions(po::options_description &options, po::options_description &hidden_options)
	{
	}
	void Command::addPositionalOptions(po::positional_options_description &positional_options)
	{
	}
	bool Command::checkOptions(po::variables_map &variable_map)
	{
		return true;
	}
	bool Command::run()
	{
		return false;
	}
	void Command::addAlias(const char *alias)
	{
		m_aliases.push_back(alias);
	}
	const vector<string>& Command::getAliases() const
	{
		return m_aliases;
	}
}
