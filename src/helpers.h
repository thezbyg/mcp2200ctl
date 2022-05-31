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
#ifndef HEADER_HELPERS_H_
#define HEADER_HELPERS_H_
#include <iostream>
#include <iomanip>
#include <boost/program_options/options_description.hpp>
namespace command_line
{
	template <typename T>
	struct BitMap
	{
		T value;
		char off, on;
		bool custom;
		BitMap():
			off('0'),
			on('1'),
			custom(false)
		{
		}
		BitMap(const T &value):
			value(value),
			off('0'),
			on('1'),
			custom(false)
		{
		}
		BitMap(const T &value, char off, char on):
			value(value),
			off(off),
			on(on),
			custom(true)
		{
		}
		operator T&()
		{
			return value;
		}
		operator const T&() const
		{
			return value;
		}
	};
	template <typename T>
	std::ostream& operator<<(std::ostream& stream, const BitMap<T> &bit_map)
	{
		char buffer[sizeof(T) * 8 + 1];
		buffer[sizeof(T) * 8] = 0;
		for (size_t i = 0; i < sizeof(T) * 8; i++){
			buffer[i] = ((bit_map.value >> i) & 1) ? bit_map.on : bit_map.off;
		}
		return stream << buffer;
	};
	inline int8_t binchar(char bin, bool &invalid)
	{
		if (bin == '0' || bin == 'f' || bin == 'F' || bin == 'o') return 0;
		if (bin == '1' || bin == 't' || bin == 'F' || bin == 'i') return 1;
		invalid = true;
		return 0;
	}
	template <typename T>
	void validate(boost::any &v, const std::vector<std::string> &values, BitMap<T> *, int)
	{
		using namespace boost::program_options;
		validators::check_first_occurrence(v);
		auto value_string = validators::get_single_string(values);
		T value = 0;
		bool invalid = false;
		if ((value_string.length() + 7) / 8 > sizeof(T)) invalid = true; // overflow
		for (size_t i = 0; i < value_string.length(); i++){
			value |= static_cast<T>(binchar(value_string[i], invalid)) << i;
		}
		if (invalid){
			throw validation_error(validation_error::invalid_option_value);
		}
		v = boost::any(BitMap<T>(value));
	}
	struct Boolean
	{
		bool value;
		Boolean(const bool &value):
			value(value)
		{
		}
	};
	std::ostream& operator<<(std::ostream& stream, const Boolean &value);
	template <typename T>
	struct HexOption
	{
		T value;
		HexOption()
		{
		}
		HexOption(const T &value):
			value(value)
		{
		}
		operator T&()
		{
			return value;
		}
		operator const T&() const
		{
			return value;
		}
	};
	inline int8_t hexchar(char hex, bool &invalid)
	{
		if (hex >= '0' && hex <= '9') return hex - '0';
		if (hex >= 'a' && hex <= 'f') return hex - 'a' + 10;
		if (hex >= 'A' && hex <= 'F') return hex - 'A' + 10;
		invalid = true;
		return 0;
	}
	template <typename T>
	void validate(boost::any &v, const std::vector<std::string> &values, HexOption<T> *, int)
	{
		using namespace boost::program_options;
		validators::check_first_occurrence(v);
		auto value_string = validators::get_single_string(values);
		T value = 0;
		bool invalid = false;
		if ((value_string.length() + 1) / 2 > sizeof(T)) invalid = true; // overflow
		for (size_t i = 0; i < value_string.length(); i++){
			value = (value << 4) | hexchar(value_string[i], invalid);
		}
		if (invalid){
			throw validation_error(validation_error::invalid_option_value);
		}
		v = boost::any(HexOption<T>(value));
	}
	struct ostream_state_saver
	{
		std::ostream *stream;
		std::ios state;
		ostream_state_saver(std::ostream &stream):
			stream(&stream),
			state(nullptr)
		{
			state.copyfmt(stream);
		}
		~ostream_state_saver()
		{
			stream->copyfmt(state);
		}
	};
#ifdef WIN32
	std::string ucsToUtf8(const std::wstring &in);
	std::wstring utf8ToUcs(const std::string &in);
#endif
}
#endif /* HEADER_HELPERS_H_ */
