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
#include "helpers.h"
#ifdef WIN32
#include <windows.h>
#endif
using namespace std;
namespace command_line
{
	std::ostream& operator<<(std::ostream& stream, const Boolean &value)
	{
		return stream << (value.value ? "on" : "off");
	};
#ifdef WIN32
	std::string ucsToUtf8(const std::wstring &in)
	{
		if (in.length() == 0)
			return "";
		stringstream reenc;
		int char_size = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)in.c_str(), in.length(), NULL, 0, NULL, NULL);
		vector<char> buffer;
		buffer.resize(char_size);
		int result1 = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)in.c_str(), in.length(), (LPSTR)&buffer.at(0), char_size, NULL, NULL);
		if (result1 != 0){
			if (buffer.at(result1 - 1) == 0) result1--;
			reenc.write((char*)&buffer.at(0), result1);
		}
		return reenc.str();
	}
	std::wstring utf8ToUcs(const std::string &in)
	{
		if (in[0] == 0) return L"";
		wstringstream reenc;
		int widechar_size = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)in.c_str(), in.length(), NULL, 0);
		vector<wchar_t> buffer;
		buffer.resize(widechar_size);
		int result1 = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)in.c_str(), in.length(), (LPWSTR)&buffer.at(0), widechar_size);
		if (result1 != 0){
			if (buffer.at(result1 - 1) == 0) result1--;
			reenc.write((wchar_t*)&buffer.at(0), result1);
		}
		return reenc.str();
	}
#endif
}
