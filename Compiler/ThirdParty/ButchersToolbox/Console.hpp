#pragma once

// Standard
#include <cstdint>
#include <cstdio>
#include <string>


namespace Console {

	/// <summary>
	/// Configures the terminal output to support UTF8 encoding
	/// </summary>
	void configureForUTF8();

	void setForegroundColor(uint8_t red, uint8_t green, uint8_t blue);

	void setBackgroundColor(uint8_t red, uint8_t green, uint8_t blue);

	void enableUnderline();

	void disableUnderline();

	void getCurrentFolder(std::string& current_folder);
}
