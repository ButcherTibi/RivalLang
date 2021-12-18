
// Header
#include "Lexer.hpp"


bool isDigit(uint8_t byte)
{
	return '0' <= byte && byte <= '9';
}

bool isSmallASCII_Letter(uint8_t byte)
{
	return 'a' <= byte && byte <= 'z';
}

bool isBigASCII_Letter(uint8_t byte)
{
	return 'A' <= byte && byte <= 'Z';
}

bool isLetter(uint8_t byte)
{
	return
		isSmallASCII_Letter(byte) ||
		isBigASCII_Letter(byte) ||
		byte == '_' ||
		byte > 127;
}

bool isSpacing(uint8_t byte)
{
	return byte == ' ' || byte == '\t' ||
		byte == '\r' || byte == '\n';
}

void FileToLex::advance()
{
	uint8_t byte = bytes[i];

	if (byte == '\n') {
		line++;
		column = 1;
	}
	else {
		column++;
	}

	i++;
}

bool FileToLex::skipToSymbolNoAdvance(uint32_t& index, char symbol)
{
	while (index < bytes.size()) {

		if (isSpacing(bytes[index]) != false) {

			if (bytes[index] == symbol) {
				return true;
			}
			else {
				return false;
			}
		}

		index++;
	}

	return false;
}

void FileToLex::lexIdentifier()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::IDENTIFIER;
	new_token.start = i;
	new_token.line = line;
	new_token.column = column;

	while (true) {

		if (isLetter(bytes[i]) || isDigit(bytes[i])) {
			advance();
		}
		else {
			new_token.end(bytes, i);
			return;
		}
	}
}

void FileToLex::lexNumber()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::NUMBER;
	new_token.start = i;
	new_token.line = line;
	new_token.column = column;

	while (i < bytes.size()) {

		uint8_t byte = bytes[i];

		if (isDigit(byte) || byte == ' ' || byte == '.') {		
			advance();
		}
		// float, double, decimal
		else if (byte == 'f' || byte == 'F' || byte == 'd') {
			advance();
			new_token.end(bytes, i);
			return;
		}
		else {
			new_token.end(bytes, i);
			return;
		}
	}
}

void FileToLex::lexHexadecimal()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::NUMBER;
	new_token.start = i;
	new_token.line = line;
	new_token.column = column;

	if (bytes[i] != '0') {
		new_token.end(bytes, i);
		return;
	}

	advance();

	if (bytes[i] != 'x') {
		new_token.end(bytes, i);
		return;
	}

	advance();

	while (i < bytes.size()) {

		uint8_t byte = bytes[i];

		if (isDigit(byte) || byte == ' ') {
			advance();
		}
		else if (('A' <= byte && byte <= 'F') || ('a' <= byte && byte <= 'f')) {
			advance();
		}
		else {
			new_token.end(bytes, i);
			return;
		}
	}
}

void FileToLex::lexBinary()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::NUMBER;
	new_token.start = i;
	new_token.line = line;
	new_token.column = column;

	if (bytes[i] != '0') {
		new_token.end(bytes, i);
		return;
	}

	advance();

	if (bytes[i] != 'b') {
		new_token.end(bytes, i);
		return;
	}

	advance();

	while (i < bytes.size()) {

		uint8_t byte = bytes[i];

		if (byte == '0' || byte == '1' || byte == ' ') {
			advance();
		}
		else {
			new_token.end(bytes, i);
			return;
		}
	}
}

void FileToLex::lexSpacing()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::SPACING;
	new_token.start = i;
	new_token.line = line;
	new_token.column = column;

	while (true) {

		if (isSpacing(bytes[i]) == false) {
			new_token.end(i);
			return;
		}

		advance();
	}
}

void FileToLex::lexString()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::STRING;
	new_token.start = i;
	new_token.line = line;
	new_token.column = column;

	uint32_t d_quotes_count = 0;
	bool special_char = false;

	while (i < bytes.size()) {

		uint8_t byte = bytes[i];

		if (special_char == false) {

			if (byte == '"') {
				d_quotes_count++;
			}
			else if (isSpacing(byte) == false && d_quotes_count % 2 == 0) {
				new_token.end(i);
				return;
			}
			// begin handling special characters such as \r \n \t
			else if (byte == '\\') {
				special_char = true;
			}
			// store character
			else if (d_quotes_count % 2 != 0) {
				new_token.value.push_back(byte);
			}
		}
		else {
			// newline
			if (byte == 'n') {
				new_token.value.push_back('\n');
			}
			// this character is legacy, useless and should no longer exist
			/*else if (byte == 'r') {
				new_token.value.push_back('\r');
			}*/
			// tab character
			else if (byte == 't') {
				new_token.value.push_back('\t');
			}
			else if (byte == '"') {
				new_token.value.push_back('"');
			}
			else {
				new_token.value.push_back('\\');
			}

			special_char = false;
		}

		advance();
	}
}

void FileToLex::lexVerbatimString()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::STRING;
	new_token.start = i;
	new_token.line = line;
	new_token.column = column;

	uint32_t d_quotes_count = 0;

	while (i < bytes.size()) {

		uint8_t byte = bytes[i];

		if (byte == '\'') {
			d_quotes_count++;
		}
		else if (isSpacing(byte) == false && d_quotes_count % 2 == 0) {
			new_token.end(i);
			return;
		}
		// store character
		else if (d_quotes_count % 2 != 0) {
			new_token.value.push_back(byte);
		}

		advance();
	}
}

void FileToLex::lexSymbol()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::SYMBOL;
	new_token.start = i;
	new_token.length = 1;
	new_token.line = line;
	new_token.column = column;
	new_token.value = bytes[i];

	advance();
}

void FileToLex::begin(std::vector<uint8_t>& new_bytes)
{
	bytes = new_bytes;
	i = 0;

	line = 1;
	column = 1;

	tokens.clear();

	enum class Mode {
		NORMAL,
		NUMBER,
		STRING,  // ""u8
		VERBATIM_STRING,  // ''u8
		STRING_WITH_PARAMS  // ``u8
	};
	Mode mode = Mode::NORMAL;

	while (i < bytes.size() - 1) {

		uint8_t byte = bytes[i];
		uint8_t next_byte = bytes[i + 1];

		switch (mode) {
		case Mode::NORMAL: {

			if (isLetter(byte)) {
				lexIdentifier();
			}
			else if (isDigit(byte)) {

				if (byte == '0' && next_byte == 'x') {
					lexHexadecimal();
				}
				else if (byte == '0' && next_byte == 'b') {
					lexBinary();
				}
				else {
					lexNumber();
				}
			}
			else if (byte == '"') {
				lexString();
			}
			else if (byte == '\'') {
				lexVerbatimString();
			}
			else if (byte == '`') {
				// lexStringWithParams
				__debugbreak();
			}
			else if (isSpacing(byte)) {
				lexSpacing();
			}
			else {
				lexSymbol();
			}
			break;
		}
		}
	}
}

void FileToLex::print()
{
	std::string token_type;

	for (Token& token : tokens) {

		switch (token.type) {
		case TokenTypes::IDENTIFIER:
			token_type = "Identifier";
			break;
		case TokenTypes::NUMBER: {
			token_type = "Number";
			break;
		}
		case TokenTypes::STRING: {
			token_type = "String";
			break;
		}
		case TokenTypes::SPACING:
			token_type = "Spacing";
			break;
		case TokenTypes::SYMBOL:
			token_type = "Symbol";
			break;
		}

		if (token.type != TokenTypes::SPACING) {
			printf("line %d column %d : %s = %s \n",
				token.line, token.column,
				token_type.c_str(), token.value.c_str());
		}
		else {
			printf("line %d column %d : %s \n",
				token.line, token.column,
				token_type.c_str());
		}
	}
}
