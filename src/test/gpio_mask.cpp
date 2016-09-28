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
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE gpio_mask
#include <boost/test/unit_test.hpp>
#include "mcp2200.h"
#include <iostream>
using namespace mcp2200;
using namespace std;
BOOST_AUTO_TEST_CASE(test1)
{
	Command command;
	command
		.setTxLedMode(LedMode::off)
		.setRxLedMode(LedMode::off)
		.setSuspend(false)
		.setUsbConfigure(false);
	BOOST_CHECK_MESSAGE(command.getIoMask() == 0xff, "mask is 0x" << hex << static_cast<int>(command.getIoMask()) << ", should be 0xff");
}
BOOST_AUTO_TEST_CASE(test2)
{
	Command command;
	command
		.setTxLedMode(LedMode::blink)
		.setRxLedMode(LedMode::off)
		.setSuspend(false)
		.setUsbConfigure(false);
	BOOST_CHECK_MESSAGE(command.getIoMask() == 0x7f, "mask is 0x" << hex << static_cast<int>(command.getIoMask()) << ", should be 0x7f");
}
BOOST_AUTO_TEST_CASE(test3)
{
	Command command;
	command
		.setTxLedMode(LedMode::off)
		.setRxLedMode(LedMode::blink)
		.setSuspend(false)
		.setUsbConfigure(false);
	BOOST_CHECK_MESSAGE(command.getIoMask() == 0xbf, "mask is 0x" << hex << static_cast<int>(command.getIoMask()) << ", should be 0xbf");
}
BOOST_AUTO_TEST_CASE(test4)
{
	Command command;
	command
		.setTxLedMode(LedMode::blink)
		.setRxLedMode(LedMode::blink)
		.setSuspend(false)
		.setUsbConfigure(false);
	BOOST_CHECK_MESSAGE(command.getIoMask() == 0x3f, "mask is 0x" << hex << static_cast<int>(command.getIoMask()) << ", should be 0x3f");
}
BOOST_AUTO_TEST_CASE(test5)
{
	Command command;
	command
		.setTxLedMode(LedMode::off)
		.setRxLedMode(LedMode::off)
		.setSuspend(true)
		.setUsbConfigure(false);
	BOOST_CHECK_MESSAGE(command.getIoMask() == 0xfe, "mask is 0x" << hex << static_cast<int>(command.getIoMask()) << ", should be 0xfe");
}
BOOST_AUTO_TEST_CASE(test6)
{
	Command command;
	command
		.setTxLedMode(LedMode::blink)
		.setRxLedMode(LedMode::blink)
		.setSuspend(true)
		.setUsbConfigure(false);
	BOOST_CHECK_MESSAGE(command.getIoMask() == 0x3e, "mask is 0x" << hex << static_cast<int>(command.getIoMask()) << ", should be 0x3e");
}
BOOST_AUTO_TEST_CASE(test7)
{
	Command command;
	command
		.setTxLedMode(LedMode::blink)
		.setRxLedMode(LedMode::blink)
		.setSuspend(true)
		.setUsbConfigure(true);
	BOOST_CHECK_MESSAGE(command.getIoMask() == 0x3c, "mask is 0x" << hex << static_cast<int>(command.getIoMask()) << ", should be 0x3c");
}
BOOST_AUTO_TEST_CASE(test8)
{
	Command command;
	command
		.setTxLedMode(LedMode::off)
		.setRxLedMode(LedMode::off)
		.setSuspend(false)
		.setUsbConfigure(true);
	BOOST_CHECK_MESSAGE(command.getIoMask() == 0xfd, "mask is 0x" << hex << static_cast<int>(command.getIoMask()) << ", should be 0xfd");
}
