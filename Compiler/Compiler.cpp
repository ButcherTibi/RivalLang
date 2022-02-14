//
//// Header
//#include "Compiler.hpp"
//
//
//#include "ThirdParty\ButchersToolbox\Filesys.hpp"
//
//
//void SourceFile::init(std::string file_path)
//{
//	std::vector<uint8_t> bytes;
//	filesys::readFile(file_path, bytes);
//
//	// Lexer
//	{
//		// curent index
//		uint32_t i = 0;
//
//		// current line and column
//		uint32_t line = 1;
//		uint32_t column = 1;
//
//		// advance to the next byte in file and keep track of line and column
//		auto advance = [&]() {
//
//			uint8_t byte = bytes[i];
//
//			if (byte == '\n') {
//				line++;
//				column = 1;
//			}
//			else {
//				column++;
//			}
//
//			i++;
//		};
//
//		while (i < bytes.size() - 1) {
//
//			uint8_t byte = bytes[i];
//			uint8_t next_byte = bytes[i + 1];
//
//			// Identifier
//			if (isLetter(byte)) {
//
//				Token& new_token = tokens.emplace_back();
//				new_token.type = TokenTypes::IDENTIFIER;
//				new_token.start = i;
//				new_token.line = line;
//				new_token.column = column;
//
//				while (true) {
//
//					if (isLetter(bytes[i]) || isDigit(bytes[i])) {
//						advance();
//					}
//					else {
//						new_token.end(bytes, i);
//						break;
//					}
//				}
//			}
//			else if (isDigit(byte)) {
//
//				// Hexadecimal Literal
//				if (byte == '0' && next_byte == 'x') {
//
//					Token& new_token = tokens.emplace_back();
//					new_token.type = TokenTypes::NUMBER;
//					new_token.start = i;
//					new_token.line = line;
//					new_token.column = column;
//
//					if (bytes[i] != '0') {
//						new_token.end(bytes, i);
//						break;
//					}
//
//					advance();
//
//					if (bytes[i] != 'x') {
//						new_token.end(bytes, i);
//						break;
//					}
//
//					advance();
//
//					while (i < bytes.size()) {
//
//						uint8_t byte = bytes[i];
//
//						if (isDigit(byte) || byte == ' ') {
//							advance();
//						}
//						else if (('A' <= byte && byte <= 'F') || ('a' <= byte && byte <= 'f')) {
//							advance();
//						}
//						else {
//							new_token.end(bytes, i);
//							break;
//						}
//					}
//				}
//				// Binary Literal
//				else if (byte == '0' && next_byte == 'b') {
//
//					Token& new_token = tokens.emplace_back();
//					new_token.type = TokenTypes::NUMBER;
//					new_token.start = i;
//					new_token.line = line;
//					new_token.column = column;
//
//					if (bytes[i] != '0') {
//						new_token.end(bytes, i);
//						break;
//					}
//
//					advance();
//
//					if (bytes[i] != 'b') {
//						new_token.end(bytes, i);
//						break;
//					}
//
//					advance();
//
//					while (i < bytes.size()) {
//
//						uint8_t byte = bytes[i];
//
//						if (byte == '0' || byte == '1' || byte == ' ') {
//							advance();
//						}
//						else {
//							new_token.end(bytes, i);
//							break;
//						}
//					}
//				}
//				// Number Literal
//				else {
//
//					Token& new_token = tokens.emplace_back();
//					new_token.type = TokenTypes::NUMBER;
//					new_token.start = i;
//					new_token.line = line;
//					new_token.column = column;
//
//					while (i < bytes.size()) {
//
//						uint8_t byte = bytes[i];
//
//						if (isDigit(byte) || byte == ' ' || byte == '.') {
//							advance();
//						}
//						// float, double, decimal
//						else if (byte == 'f' || byte == 'F' || byte == 'd') {
//							advance();
//							new_token.end(bytes, i);
//							break;
//						}
//						else {
//							new_token.end(bytes, i);
//							break;
//						}
//					}
//				}
//			}
//			// String Literal
//			else if (byte == '"') {
//
//				Token& new_token = tokens.emplace_back();
//				new_token.type = TokenTypes::STRING;
//				new_token.start = i;
//				new_token.line = line;
//				new_token.column = column;
//
//				uint32_t d_quotes_count = 0;
//				bool special_char = false;
//
//				while (i < bytes.size()) {
//
//					uint8_t byte = bytes[i];
//
//					if (special_char == false) {
//
//						if (byte == '"') {
//							d_quotes_count++;
//						}
//						else if (isSpacing(byte) == false && d_quotes_count % 2 == 0) {
//							new_token.end(i);
//							break;
//						}
//						// begin handling special characters such as \r \n \t
//						else if (byte == '\\') {
//							special_char = true;
//						}
//						// store character
//						else if (d_quotes_count % 2 != 0) {
//							new_token.value.push_back(byte);
//						}
//					}
//					else {
//						// newline
//						if (byte == 'n') {
//							new_token.value.push_back('\n');
//						}
//						// this character is legacy, useless and should no longer exist
//						/*else if (byte == 'r') {
//							new_token.value.push_back('\r');
//						}*/
//						// tab character
//						else if (byte == 't') {
//							new_token.value.push_back('\t');
//						}
//						else if (byte == '"') {
//							new_token.value.push_back('"');
//						}
//						else {
//							new_token.value.push_back('\\');
//						}
//
//						special_char = false;
//					}
//
//					advance();
//				}
//			}
//			// Verbatim String Literal
//			else if (byte == '\'') {
//
//				Token& new_token = tokens.emplace_back();
//				new_token.type = TokenTypes::STRING;
//				new_token.start = i;
//				new_token.line = line;
//				new_token.column = column;
//
//				uint32_t d_quotes_count = 0;
//
//				while (i < bytes.size()) {
//
//					uint8_t byte = bytes[i];
//
//					if (byte == '\'') {
//						d_quotes_count++;
//					}
//					else if (isSpacing(byte) == false && d_quotes_count % 2 == 0) {
//						new_token.end(i);
//						break;
//					}
//					// store character
//					else if (d_quotes_count % 2 != 0) {
//						new_token.value.push_back(byte);
//					}
//
//					advance();
//				}
//			}
//			// Parameterized String Literal
//			else if (byte == '`') {
//
//				// lexStringWithParams
//				__debugbreak();
//			}
//			// Spacing 
//			else if (isSpacing(byte)) {
//
//				Token& new_token = tokens.emplace_back();
//				new_token.type = TokenTypes::SPACING;
//				new_token.start = i;
//				new_token.line = line;
//				new_token.column = column;
//
//				while (i < bytes.size()) {
//
//					if (isSpacing(bytes[i]) == false) {
//						new_token.end(i);
//						break;
//					}
//
//					advance();
//				}
//			}
//			// Symbol
//			else {
//				Token& new_token = tokens.emplace_back();
//				new_token.type = TokenTypes::SYMBOL;
//				new_token.start = i;
//				new_token.length = 1;
//				new_token.line = line;
//				new_token.column = column;
//				new_token.value = bytes[i];
//
//				advance();
//			}
//		}
//	}
//}
//
//void Compiler::addSourceFile(std::string file_path)
//{
//	SourceFile& source_file = source_files.emplace_back();
//
//	std::vector<uint8_t> bytes;
//	filesys::readFile(file_path, bytes);
//}