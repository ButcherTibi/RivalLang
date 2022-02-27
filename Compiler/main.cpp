
// Standard
#include <cstdio>

// Toolbox
#include "ThirdParty\ButchersToolbox\Console.hpp"
#include "ThirdParty\ButchersToolbox\utf8_string.hpp"
#include "ThirdParty\ButchersToolbox\Filesys.hpp"
#include "Lexer/Lexer.hpp"
#include "Parser/Parser.hpp"


int main(int argument_count, char* argv[])
{
	Console::configureForUTF8();

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

			// File Path
			std::string current_folder;
			Console::getCurrentFolder(current_folder);
			
			std::string file_name = "main.txt";
			current_folder.append("\\");
			current_folder.append(file_name);

			// Read File
			std::vector<uint8_t> bytes;
			filesys::readFile(current_folder, bytes);
			bytes.push_back('\0');

			Parser parser;
			parser.init();
			parser.parseFile(bytes, file_name);
			parser.resolve();

			printf("\n");
			PrintAST_TreeSettings settings;
			settings.show_code_selections = false;
			parser.printAST(settings);

			printf("\n");
			parser.printDeclarations();
		}
	}

	// Reset as not to poison new prompt
	Console::setForegroundColor(200, 200, 200);
	Console::setBackgroundColor(0, 0, 0);
	Console::disableUnderline();
	return 0;
}
