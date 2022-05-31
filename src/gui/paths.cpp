#include "paths.h"
#include <vector>
namespace fs = std::filesystem;
#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#include <string.h>
#else
#include <glib.h>
#include <unistd.h>
#endif
namespace paths
{
#ifdef WIN32
	std::filesystem::path getProgramFile()
	{
		std::vector<wchar_t> path;
		path.resize(1024);
		DWORD result = GetModuleFileName(NULL, &path[0], static_cast<DWORD>(path.size()));
		while (result == path.size()){
			path.resize(path.size() * 2);
			result = GetModuleFileName(NULL, &path[0], static_cast<DWORD>(path.size()));
		}
		if (result == 0) throw std::runtime_error("GetModuleFileName() failed");
		return fs::path(std::wstring(path.begin(), path.begin() + result));
	}
	std::filesystem::path getConfigurationPath()
	{
		PWSTR path;
		HRESULT result = SHGetKnownFolderPath(&FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &path);
		if (result != S_OK){
			throw std::runtime_error("SHGetKnownFolderPath() failed");
		}
		fs::path fs_path(std::wstring(path, path + wcslen(path)));
		CoTaskMemFree(path);
		return fs_path;
	}
#else
	std::filesystem::path getProgramFile()
	{
		std::vector<char> buffer;
		buffer.resize(1024);
		for (;;){
			int nchars = readlink("/proc/self/exe", &buffer.front(), buffer.size());
			if (nchars < 0) throw std::runtime_error("Reading /proc/self/exe failed");
			if ((size_t)nchars < buffer.size()){
				return fs::path(std::string(buffer.begin(), buffer.begin() + nchars));
			}
			buffer.resize(buffer.size() * 2);
		}
		return fs::path();
	}
	std::filesystem::path getConfigurationPath()
	{
		const char *path = g_get_user_config_dir();
		return fs::path(std::string(path, path + strlen(path)));
	}
#endif
	std::filesystem::path getProgramPath()
	{
		return getProgramFile().remove_filename();
	}
	std::string getProgramPathString()
	{
		return getProgramPath().string();
	}
}
