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
#ifndef COMMAND_H_
#define COMMAND_H_
#include <string>
#include <vector>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/positional_options.hpp>
namespace command_line
{
	struct Command
	{
		Command(const char *name, const char *description);
		Command(const char *name, const char *description, const char *command_pattern);
		virtual ~Command();
		const char *getName() const;
		const char *getDescription() const;
		const char *getCommandPattern() const;
		virtual void addOptions(boost::program_options::options_description &options, boost::program_options::options_description &hidden_options);
		virtual void addPositionalOptions(boost::program_options::positional_options_description &positional_options);
		virtual bool checkOptions(boost::program_options::variables_map &variable_map);
		virtual bool run();
		void addAlias(const char *alias);
		const std::vector<std::string>& getAliases() const;
		private:
		std::string m_name;
		std::string m_description;
		std::string m_command_pattern;
		std::vector<std::string> m_aliases;
	};
}
#endif /* COMMAND_H_ */
