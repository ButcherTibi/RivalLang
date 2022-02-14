
// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Header
#include "Console.hpp"


namespace Console {

	void configureForUTF8()
	{
		// Set output mode to handle virtual terminal sequences
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

		DWORD dwMode = 0;
		GetConsoleMode(handle, &dwMode);

		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(handle, dwMode);

		// Set Console Output to UTF-8
		SetConsoleOutputCP(CP_UTF8);
	}

	void setForegroundColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		printf("\x1b[38;2;%d;%d;%dm", red, green, blue);
	}

	void setBackgroundColor(uint8_t red, uint8_t green, uint8_t blue)
	{
		printf("\x1b[48;2;%d;%d;%dm", red, green, blue);
	}

	void enableUnderline()
	{
		printf("\x1b[4m");
	}

	void disableUnderline()
	{
		printf("\x1b[24m");
	}

	void getCurrentFolder(std::string& current_folder)
	{
		current_folder.resize(GetCurrentDirectoryA(0, nullptr));
		GetCurrentDirectoryA((uint32_t)current_folder.size(), current_folder.data());

		// don't need \\0
		current_folder.pop_back();
	}
}
