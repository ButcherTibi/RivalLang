
// Standard
#include <cstdio>

// Mine
#include "Toolbox\Console.hpp"
#include "Toolbox\Filesys.hpp"
#include "Toolbox\utf_string.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"


int main(int argument_count, char* argv[])
{
	// Configure Console
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

	if (argument_count > 1) {

		std::vector<utf8string> params;
		params.resize(argument_count);

		for (int32_t i = 0; i < argument_count; i++) {
			params[i] = (const char8_t *)argv[i];
		}

		// runtime
		if (params[1] == u8"run") {

			printf("run \n");
		}
		// compiler
		else if (params[1] == u8"compile") {

			// read file
			std::string current_folder;
			Console::getCurrentFolder(current_folder);

			std::vector<uint8_t> bytes;
			current_folder.append("\\main.txt");
			filesys::readFile(current_folder, bytes);

			bytes.push_back('\0');
			printf("%s \n", bytes.data());

			// Lexer
			FileToLex file_to_lex;
			file_to_lex.begin(bytes);
			file_to_lex.print();

			// Parser
			Parser parser;
			parser.begin(file_to_lex);
			printf("\n");
			parser.printTree();
		}
	}

	// Reset as not to poison new prompt
	Console::setForegroundColor(200, 200, 200);
	Console::setBackgroundColor(0, 0, 0);
	Console::disableUnderline();
	return 0;
}
