
// Standard
#include <cstdio>
#include <array>

// Toolbox
#include "ThirdParty\ButchersToolbox\Console.hpp"
#include "ThirdParty\ButchersToolbox\utf8_string.hpp"
#include "ThirdParty\ButchersToolbox\Filesys.hpp"
#include "Resolve/Resolve.hpp"


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

			std::array<std::string, 1> files = {
				file_name
			};
			AST_NodeIndex ast_source_file;

			Resolve front_end;
			front_end.init();
			front_end.lexer.lexFile(0, bytes);
			front_end.parseSourceFile(ast_source_file);
			front_end.resolve();

			PrintAST_TreeSettings settings;
			settings.show_code_selections = false;
			front_end.printAST(settings);

			printf("\n");
			front_end.printDeclarations();

			// Errors
			if (front_end.messages.size() > 0) {

				printf("\nErrors: \n");

				for (CompilerMessage& message : front_end.messages) {

					for (MessageRow& row : message.rows) {

						printf("(%d, %d) %s \n",
							row.selection.start.line, row.selection.start.column,
							row.text.c_str());
					}
				}
			}
		}
	}

	// Reset as not to poison new prompt
	Console::setForegroundColor(200, 200, 200);
	Console::setBackgroundColor(0, 0, 0);
	Console::disableUnderline();
	return 0;
}
