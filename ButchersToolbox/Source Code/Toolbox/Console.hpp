#pragma once

// Standard
#include <cstdint>
#include <cstdio>
#include <string>


namespace Console {

	// NOTE: styling the console also applies for the first prompt after the program end

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
