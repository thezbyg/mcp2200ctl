/*
Copyright (c) 2016-2018, Albertas Vyšniauskas
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
#include "types.h"
#include <sstream>
#include <iomanip>
using namespace std;
namespace gui {
	Uint16HexValue::Uint16HexValue(const char *name, size_t offset, const uint16_t &default_value):
		Value(name, offset, "uint16_hex"),
		default_value(default_value)
	{
	}
	Uint16HexValue::~Uint16HexValue()
	{
	}
	void Uint16HexValue::setDefault(void *object) const
	{
		*this->getValuePointer(object) = default_value;
	}
	Uint16HexAdapter::Uint16HexAdapter():
		Adapter("uint16_hex")
	{}
	Uint16HexAdapter::~Uint16HexAdapter()
	{
	}
	void Uint16HexAdapter::save(const void *object, const mapping::BaseValue &value, Json::Value &output)
	{
		stringstream s;
		s << setfill('0') << hex << setw(4) << value.get<uint16_t>(object);
		output[value.getName()] = s.str();
	}
	void Uint16HexAdapter::load(void *object, const mapping::BaseValue &value, const Json::Value &input)
	{
		auto &name = value.getName();
		if (input.isMember(name) && input[name].isConvertibleTo(Json::stringValue)){
			stringstream s(input[name].asString());
			uint16_t v;
			s >> hex >> v;
			value.set<uint16_t>(object, static_cast<const uint16_t&>(v));
		}else if (input.isMember(name) && input[name].isConvertibleTo(Json::intValue)){
			value.set<uint16_t>(object, input[name].asInt());
		}else{
			value.setDefault(object);
		}
	}
	StringValue::StringValue(const char *name, size_t offset, const std::string &default_value):
		Value(name, offset, "string"),
		default_value(default_value)
	{
	}
	StringValue::~StringValue()
	{
	}
	void StringValue::setDefault(void *object) const
	{
		*this->getValuePointer(object) = default_value;
	}
	StringAdapter::StringAdapter():
		Adapter("string")
	{}
	StringAdapter::~StringAdapter()
	{
	}
	void StringAdapter::save(const void *object, const mapping::BaseValue &value, Json::Value &output)
	{
		output[value.getName()] = value.get<string>(object);
	}
	void StringAdapter::load(void *object, const mapping::BaseValue &value, const Json::Value &input)
	{
		auto &name = value.getName();
		if (input.isMember(name) && input[name].isConvertibleTo(Json::stringValue)){
			value.set<string>(object, input[name].asString());
		}else{
			value.setDefault(object);
		}
	}
}
