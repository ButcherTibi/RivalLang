
// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Header
#include "Filesys.hpp"

// Windows
#include <intrin.h>

// Standard
#include <cstdio>

// Mine
#include "DebugUtils.hpp"
#include "WindowsSpecific.hpp"


using namespace filesys;


uint32_t filesys::max_path_length = 2024;


void filesys::readFile(std::string& absolute_path, std::vector<uint8_t>& r_bytes)
{
	std::wstring filename_win;
	filename_win.resize(absolute_path.size() * 2);

	size_t unused;

	mbstowcs_s(
		&unused,
		filename_win.data(), filename_win.size(),
		absolute_path.data(), absolute_path.size() * 2);

	win32::Handle file_handle = CreateFile(filename_win.c_str(),
		GENERIC_READ, // desired acces
		0,  // share mode
		NULL,  // security atributes
		OPEN_EXISTING,  // disposition
		FILE_FLAG_SEQUENTIAL_SCAN, // flags and atributes
		NULL  // template
	);

	if (file_handle.isValid() == false) {

		printf("%s failed to a create file handle for path = %s \n",
			code_location,
			absolute_path.c_str());
		__debugbreak();
	}

	// find file size
	LARGE_INTEGER file_size;
	if (GetFileSizeEx(file_handle.handle, &file_size) == false) {

		printf("%s failed to find file size for path = %s \n",
			code_location,
			absolute_path.c_str());
		__debugbreak();
	}
	r_bytes.resize(file_size.QuadPart);

	// read file
	DWORD bytes_read;

	auto result = ReadFile(
		file_handle.handle,
		r_bytes.data(),
		(DWORD)file_size.QuadPart,
		&bytes_read,
		NULL);

	if (result == false)
	{
		printf("%s failed to read file for path =  %s \n",
			code_location,
			absolute_path.c_str());
		__debugbreak();
	}
}

//static uint32_t findEntryCount(std::string path)
//{
//	size_t last = path.size() - 1;
//	uint32_t entry_count = 0;
//	bool was_something = false;
//
//	for (size_t i = 0; i < path.size(); i++) {
//
//		if (i == last &&
//			(path[i] != '\\' && path[i] != '/'))
//		{
//			// path = "e" or path = "entry"
//			entry_count++;
//		}
//		else if (path[i] == '\\' || path[i] == '/') {
//
//			// path = "entry\"
//			if (was_something) {
//
//				entry_count++;
//				was_something = false;
//			}
//			// path = "\\\\\"
//		}
//		else {
//			was_something = true;
//		}
//	}
//	return entry_count;
//}
//
//static void pushPathToEntries(std::vector<std::string>& existing, std::string& path)
//{
//	existing.reserve(existing.size() + findEntryCount(path));
//
//	size_t last = path.size() - 1;
//	std::string entry;
//	bool was_something = false;
//
//	for (uint32_t i = 0; i < path.size(); i++) {
//
//		if (i == last &&
//			(path[i] != '\\' && path[i] != '/'))
//		{
//			// path = "e" or path = "entry"
//			entry.push_back(path[i]);
//			existing.push_back(entry);
//		}
//		else if (path[i] == '\\' || path[i] == '/') {
//
//			// path = "entry\"
//			if (was_something) {
//
//				existing.push_back(entry);
//				entry.clear();
//				was_something = false;
//			}
//			// path = "\\\\\"
//		}
//		else {
//			was_something = true;
//			entry.push_back(path[i]);
//		}
//	}
//}
//
//Path::Path(std::string path)
//{
//	entries.clear();
//	pushPathToEntries(this->entries, path);
//}
//
//Path::Path(std::string& path)
//{
//	entries.clear();
//	pushPathToEntries(this->entries, path);
//}
//
//ErrStack Path::recreateFromRelativePath(std::string path)
//{
//	ErrStack err_stack;
//
//	std::string solution_path;
//	checkErrStack1(getSolutionPath(solution_path));
//
//	solution_path.append(path);
//
//	push_back(solution_path);
//
//	return ErrStack();
//}
//
//bool Path::hasExtension(std::string extension)
//{
//	std::string last = entries.back();
//	size_t dot_pos = last.find_last_of('.');
//	std::string entry_ext = last.substr(dot_pos + 1, extension.size());
//
//	return extension == entry_ext;
//}
//
//void Path::push_back(std::string path)
//{
//	pushPathToEntries(this->entries, path);
//}
//
//void Path::pop_back(size_t count)
//{
//	for (size_t i = 0; i < count; i++) {
//		entries.pop_back();
//	}
//}
//
//void Path::push_front(std::string path)
//{
//	std::vector<std::string> append = this->entries;
//
//	this->entries.clear();
//	pushPathToEntries(this->entries, path);
//
//	for (std::string entry : append) {
//		this->entries.push_back(entry);
//	}
//}
//
//void Path::pop_front(size_t count)
//{
//	this->entries.erase(entries.begin(), entries.begin() + count);
//}
//
//void Path::erase(size_t start, size_t end)
//{
//	this->entries.erase(entries.begin() + start, entries.begin() + end);
//}
//
//std::string Path::toWindowsPath()
//{
//	std::string path;
//
//	if (this->entries.size()) {
//
//		size_t last = entries.size() - 1;
//		for (size_t i = 0; i < entries.size(); i++) {
//
//			for (char letter : entries[i]) {
//				path.push_back(letter);
//			}
//
//			if (i != last) {
//				path.push_back('\\');
//			}
//		}
//	}
//	return path;
//}
//
//template<typename T>
//ErrStack Path::readOnce(std::vector<T>& content)
//{
//	// create file handle
//	std::string filename_vec = toWindowsPath();
//
//	std::wstring filename_win;
//	filename_win.resize(filename_vec.size());
//
//	mbstowcs((wchar_t*)filename_win.c_str(), filename_vec.c_str(), filename_vec.length());
//
//	HANDLE file_handle = CreateFile(filename_win.c_str(),
//		GENERIC_READ, // desired acces
//		0,  // share mode
//		NULL,  // security atributes
//		OPEN_EXISTING,  // disposition
//		FILE_ATTRIBUTE_NORMAL, // flags and atributes
//		NULL);
//
//	if (file_handle == INVALID_HANDLE_VALUE) {
//		return ErrStack(code_location,
//			"failed to create file handle for path = " + this->toWindowsPath());
//	}
//
//	// find file size
//	LARGE_INTEGER file_size;
//	if (!GetFileSizeEx(file_handle, &file_size)) {
//
//		CloseHandle(file_handle);
//		return ErrStack(code_location,
//			"failed to find file size for path = " + this->toWindowsPath());
//	}
//	content.resize(file_size.QuadPart);
//
//	// read file
//	DWORD bytes_read;
//
//	if (!ReadFile(file_handle, content.data(), (DWORD)file_size.QuadPart, &bytes_read, NULL)) {
//
//		CloseHandle(file_handle);
//		return ErrStack(code_location,
//			"failed to read path = " + this->toWindowsPath());
//	}
//
//	CloseHandle(file_handle);
//	return ErrStack();
//}
//template ErrStack Path::readOnce(std::vector<char>& content);
//template ErrStack Path::readOnce(std::vector<uint8_t>& content);
//
//
//void File::create(Path& path)
//{
//	assert_cond(file_path.length() == 0);
//
//	this->file_path = path.toWindowsPath();
//}
//
//ErrStack File::openForParsing()
//{
//	//if (_file_handle.isValid()) {
//	//	CloseHandle(_file_handle.handle);
//	//	_file_handle.handle = INVALID_HANDLE_VALUE;
//	//}
//
//	//_file_handle = CreateFile(file_path.data(),
//	//	GENERIC_READ, // desired acces
//	//	FILE_SHARE_READ | FILE_SHARE_WRITE,  // share mode
//	//	NULL,  // security atributes
//	//	OPEN_EXISTING,  // disposition
//	//	FILE_FLAG_SEQUENTIAL_SCAN, // flags and atributes
//	//	NULL);
//
//	//if (_file_handle.isValid() == false) {
//	//	return ErrStack(code_location,
//	//		"failed to open file handle for path = " + file_path + " " + getLastError());
//	//}
//
//	return ErrStack();
//}
//
//ErrStack File::size(size_t& r_byte_count)
//{
//	auto get_size = [&](Handle& file_handle, size_t& r_size) -> ErrStack {
//
//		LARGE_INTEGER file_size;
//
//		if (!GetFileSizeEx(file_handle.handle, &file_size)) {
//			return ErrStack(code_location,
//				"failed to find file size for path = " + file_path + " " + getLastError());
//		}
//		r_size = file_size.QuadPart;
//
//		return ErrStack();
//	};
//
//	// create a temporary file handle
//	if (_file_handle.isValid() == false) {
//
//		std::wstring file_path_wstr;
//		file_path_wstr.resize(file_path.size());
//
//		mbstowcs((wchar_t*)file_path_wstr.c_str(), file_path.c_str(), file_path.length());
//
//		// create file handle
//		Handle file_handle = CreateFile(file_path_wstr.c_str(),
//			GENERIC_READ, // desired acces
//			0,  // share mode
//			NULL,  // security atributes
//			OPEN_EXISTING,  // disposition
//			FILE_ATTRIBUTE_NORMAL, // flags and atributes
//			NULL);
//
//		if (file_handle.isValid() == false) {
//			return ErrStack(code_location,
//				"failed to create file handle for path = " + file_path + " " + getLastError());
//		}
//
//		return get_size(file_handle, r_byte_count);
//	}
//
//	return get_size(this->_file_handle, r_byte_count);
//}
//
//ErrStack File::getLastWrite(uint32_t& r_day, uint32_t& r_hour, uint32_t& r_minute, uint32_t& r_second)
//{
//	ErrStack err_stack;
//
//	FILETIME last_write_time;
//
//	if (GetFileTime(_file_handle.handle, nullptr, nullptr, &last_write_time) == false) {
//		return ErrStack(code_location,
//			"failed to read file time of file = " + file_path + " " + getLastError());
//	}
//
//	SYSTEMTIME system_time;
//	FileTimeToSystemTime(&last_write_time, &system_time);
//
//	r_day = system_time.wDay;
//	r_hour = system_time.wHour;
//	r_minute = system_time.wMinute;
//	r_second = system_time.wSecond;
//
//	return err_stack;
//}
//
//template<typename T>
//ErrStack File::read(std::vector<T>& content)
//{
//	ErrStack err_stack;
//
//	// read file
//	size_t bytes_to_read;
//	checkErrStack1(size(bytes_to_read));
//
//	//uint32_t bytes_to_read_dword = ;
//	DWORD bytes_read;
//
//	if (ReadFile(_file_handle.handle, content.data(), (uint32_t)bytes_to_read, &bytes_read, NULL) == false) {
//		return ErrStack(code_location,
//			"failed to read whole file = " + file_path + " " + getLastError());
//	}
//
//	return err_stack;
//}
//template ErrStack File::read(std::vector<char>& content);
//template ErrStack File::read(std::vector<uint8_t>& content);
//
//
//ErrStack io::getSolutionPath(std::string& r_path)
//{
//	r_path.resize(max_path);
//
//	if (!GetModuleFileNameA(NULL, r_path.data(), (DWORD)max_path)) {
//		return ErrStack(code_location,
//			"could not obtain solution path " + getLastError());
//	}
//
//	// trim excess
//	for (uint32_t i = 0; i < r_path.size(); i++) {
//		if (r_path[i] == '\0') {
//			r_path.erase(i + 1, r_path.size() - (i + 1));
//			break;
//		}
//	}
//
//	// remove last 3 entries
//	uint32_t slash_pos = 0;
//	uint32_t slash_count = 0;
//	for (int32_t i = r_path.size() - 1; i >= 0; i--) {
//
//		char& c = r_path[i];
//
//		if (c == '\\' || c == '/') {
//			slash_count++;
//
//			if (slash_count == 3) {
//				slash_pos = i;
//				break;
//			}
//		}
//	}
//	r_path.erase(slash_pos + 1, r_path.size() - (slash_pos + 1));
//
//	// convert from windows '\' to linux '/'
//	for (char& c : r_path) {
//		c = c == '\\' ? '/' : c;
//	}
//
//	return ErrStack();
//}
//
//template<typename T>
//ErrStack io::readFile(std::string& path, std::vector<T>& content)
//{
//	Path file = std::string(path);  // why won't it call my reference constructor ?
//	return file.readOnce(content);
//}
//template ErrStack io::readFile(std::string& path, std::vector<char>& content);
//template ErrStack io::readFile(std::string& path, std::vector<uint8_t>& content);
//
//
//template<typename T>
//ErrStack io::readLocalFile(std::string path, std::vector<T>& content)
//{
//	ErrStack err_stack;
//
//	std::string solution_path;
//	checkErrStack1(getSolutionPath(solution_path));
//
//	solution_path.append(path);
//
//	return readFile(solution_path, content);
//}
//template ErrStack io::readLocalFile(std::string path, std::vector<char>& content);
//template ErrStack io::readLocalFile(std::string path, std::vector<uint8_t>& content);
