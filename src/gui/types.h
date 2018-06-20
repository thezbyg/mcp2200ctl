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
#include "mapping.h"
#include <json/json.h>
#include <string>
#include <stdint.h>
namespace gui {
	struct Uint16HexValue: public mapping::Value<uint16_t>
	{
		uint16_t default_value;
		Uint16HexValue(const char *name, size_t offset, const uint16_t &default_value = 0);
		virtual ~Uint16HexValue();
		virtual void setDefault(void *object) const;
	};
	struct Uint16HexAdapter: public mapping::Adapter<Json::Value>
	{
		Uint16HexAdapter();
		virtual ~Uint16HexAdapter();
		virtual void save(const void *object, const mapping::BaseValue &value, Json::Value &output);
		virtual void load(void *object, const mapping::BaseValue &value, const Json::Value &input);
	};
	struct StringValue: public mapping::Value<std::string>
	{
		std::string default_value;
		StringValue(const char *name, size_t offset, const std::string &default_value = "");
		virtual ~StringValue();
		virtual void setDefault(void *object) const;
	};
	struct StringAdapter: public mapping::Adapter<Json::Value>
	{
		StringAdapter();
		virtual ~StringAdapter();
		virtual void save(const void *object, const mapping::BaseValue &value, Json::Value &output);
		virtual void load(void *object, const mapping::BaseValue &value, const Json::Value &input);
	};
}
