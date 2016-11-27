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
#include "mcp2200ctl.h"
#include "mcp2200.h"
#include "command.h"
#include "list_command.h"
#include "get_command.h"
#include "set_command.h"
#include "get_eeprom_command.h"
#include "set_eeprom_command.h"
#include "configure_command.h"
#include "describe_command.h"
#include "helpers.h"
#include "version.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <boost/program_options/option.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/config.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#ifdef WIN32
#include <windows.h>
#endif
namespace po = boost::program_options;
using namespace std;
namespace command_line
{
	const static char *program_name = "mcp2200ctl";
	const static char *program_description = "mcp2200ctl - MCP2200 control and query program";
	struct HelpCommand: public Command
	{
		Program *m_program;
		HelpCommand(Program *program):
			Command("help", "print this help message"),
			m_program(program)
		{
		}
		virtual ~HelpCommand()
		{
		}
		virtual bool run()
		{
			m_program->printHelp();
			return true;
		}
	};
	Program::Program()
	{
		addCommand(make_shared<ListCommand>());
		addCommand(make_shared<GetCommand>());
		addCommand(make_shared<SetCommand>());
		addCommand(make_shared<ConfigureCommand>());
		addCommand(make_shared<DescribeCommand>());
		addCommand(make_shared<GetEepromCommand>());
		addCommand(make_shared<SetEepromCommand>());
		addCommand(make_shared<HelpCommand>(this));
	}
	void Program::addCommand(shared_ptr<Command> command)
	{
		m_command_map.insert(CommandPair(command->getName(), command));
		for (auto alias: command->getAliases()){
			m_command_map.insert(CommandPair(alias, command));
		}
	}
	shared_ptr<Command> Program::findCommand(const char *name)
	{
		auto i = m_command_map.find(name);
		if (i == m_command_map.end()){
			return shared_ptr<Command>();
		}else{
			return (*i).second;
		}
	}
	pair<bool, std::string> Program::getCommandName(const vector<string> &command_line)
	{
		vector<string> positionals;

		po::options_description options("");
		addGlobalOptions(options);
		options.add_options()
			("positional_commands",  po::value<vector<string>>(&positionals), "")
		;
		po::positional_options_description positional_options;
		positional_options.add("positional_commands", -1);

		try{
			auto parsed = po::command_line_parser(command_line).options(options).positional(positional_options).allow_unregistered().run();
			po::variables_map vm;
			po::store(parsed, vm);
			po::notify(vm);
			if (m_version){
				cout << program_name << " " << version::version << "\n";
				return pair<bool, string>(false, "");
			}
			for (size_t i = 0; i < positionals.size(); i++){
				if (findCommand(positionals[i].c_str())){
					return pair<bool, string>(true, positionals[i]);
				}
			}
			for (size_t i = 0; i < positionals.size(); i++){
				return pair<bool, string>(true, positionals[i]);
			}
			return pair<bool, string>(true, "");
		}catch(const exception &e){
			return pair<bool, string>(true, "");
		}
	}
	void Program::addGlobalOptions(po::options_description &options)
	{
		options.add_options()
			("version,v", po::value<bool>(&m_version)->default_value(false)->zero_tokens(), "print program version information")
			("help,h", "display help and exit")
		;
	}
	void Program::printHelp()
	{
		cout << program_description << "\n";
		po::options_description global_options, options, hidden_options;
		addGlobalOptions(global_options);
		cout << "Usage:" << "\n  " << program_name << " COMMAND [OPTIONS]" << "\n";
		cout << "Commands:" << "\n";
		for (auto command: m_command_map){
			cout << "  " << std::left << std::setw(13) << command.first << " " << command.second->getDescription() << "\n";
		}
		cout << "Options:" << "\n" << global_options;
	}
	void Program::printHelp(Command &command)
	{
		cout << program_description << "\n";
		po::options_description global_options, options, hidden_options;
		addGlobalOptions(global_options);
		command.addOptions(options, hidden_options);
		cout << "Usage:" << "\n  " << program_name << " " << command.getName() << " [OPTIONS] [GLOBAL OPTIONS] " << command.getCommandPattern() << "\n";
		cout << "Options:" << "\n" << options;
		cout << "Global options:" << "\n" << global_options;
	}
	int Program::run(int argc, char **argv)
	{
#ifdef WIN32
			auto command_line = po::split_winmain(ucsToUtf8(wstring(GetCommandLine())));
			command_line.erase(command_line.begin());
#else
			vector<string> command_line;
			command_line.resize(argc - 1);
			for (int i = 1; i < argc; i++) {
				command_line[i - 1] = argv[i];
			}
#endif
		auto command_name = getCommandName(command_line);
		if (!command_name.first) return EXIT_FAILURE;

		if (command_name.first && command_name.second == ""){
			printHelp();
			return EXIT_FAILURE;
		}
		auto command = findCommand(command_name.second.c_str());
		if (!command){
			cerr << program_name << ": " << "unknown command '" << command_name.second << "'" << "\n";
			printHelp();
			return EXIT_FAILURE;
		}

		try{
			po::options_description all_options, global_options, options, hidden_options;
			po::positional_options_description positional_options;
			string command_confirm;
			addGlobalOptions(global_options);
			hidden_options.add_options()
				("command",  po::value<string>(&command_confirm), "")
			;
			positional_options.add("command", 1);
			command->addOptions(options, hidden_options);
			command->addPositionalOptions(positional_options);
			all_options.add(global_options).add(options).add(hidden_options);
			auto parsed = po::command_line_parser(command_line).options(all_options).positional(positional_options).run();
			po::variables_map vm;
			po::store(parsed, vm);
			po::notify(vm);
			if (vm.count("help")) {
				printHelp(*command);
				return EXIT_SUCCESS;
			}
			if (!command->checkOptions(vm)){
				printHelp(*command);
				return EXIT_FAILURE;
			}
			return command->run();
		}catch(const exception &e){
			cerr << program_name << ": " << e.what() << "\n";
			return EXIT_FAILURE;
		}
	}
}
