
// Header
#include "Lexer.hpp"


std::string toStringTokenTypes(TokenTypes token_type)
{
	switch (token_type) {
	case TokenTypes::IDENTIFIER:
		return "Identifier";

		// Integer
	case TokenTypes::i32:
		return "i32";
	case TokenTypes::i64:
		return "i64";
	case TokenTypes::u32:
		return "u32";
	case TokenTypes::u64:
		return "u64";

		// Float point
	case TokenTypes::f32:
		return "f32";
	case TokenTypes::f64:
		return "f64";

	case TokenTypes::number:
		return "number";

	case TokenTypes::STRING:
		return "String";
	case TokenTypes::SPACING:
		return "Spacing";
	case TokenTypes::SYMBOL:
		return "Symbol";
	}

	throw;
}

void Token::end(uint32_t i_to_next_token)
{
	length = i_to_next_token - start;
}

void Token::end(std::vector<uint8_t>& bytes, uint32_t i_to_next_token)
{
	length = i_to_next_token - start;

	value.resize(length);
	std::memcpy(value.data(), bytes.data() + start, length);
}

void Token::endSuffixedLiteral(std::vector<uint8_t>& bytes, uint32_t end_i, uint32_t suffix_length)
{
	length = end_i - start;

	uint32_t value_length = length - suffix_length;
	value.resize(value_length);
	std::memcpy(value.data(), bytes.data() + start, value_length);
}

bool Token::isSpacing()
{
	return type == TokenTypes::SPACING;
}

bool Token::isNumberLike()
{
	return
		type == TokenTypes::i32 ||
		type == TokenTypes::i64 ||
		type == TokenTypes::u32 ||
		type == TokenTypes::u64 ||
		type == TokenTypes::f32 ||
		type == TokenTypes::f64;
}

bool Token::isSymbol()
{
	return type == TokenTypes::SYMBOL;
}

bool Token::isSymbol(std::string other)
{
	return type == TokenTypes::SYMBOL && value == other;
}

bool Token::isExpressionSign()
{
	if (type == TokenTypes::SYMBOL) {

		if (value == "+" ||
			value == "-" ||
			value == "*" ||
			value == "/" ||
			value == "%")
		{
			return true;
		}
	}

	return false;
}

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

void Lexer::advance()
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

bool Lexer::skipToSymbolNoAdvance(uint32_t& index, char symbol)
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

void Lexer::lexIdentifier()
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

void Lexer::lexNumber()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::i32;
	new_token.start = i;
	new_token.line = line;
	new_token.column = column;

	while (i < bytes.size()) {

		uint8_t byte = bytes[i];

		if (isDigit(byte) || byte == ' ') {		
			advance();
		}
		// 32 bit float point
		else if (byte == '.') {
			new_token.type = TokenTypes::f32;
			advance();
		}

		// 32 bit unsigned integer
		else if (byte == 'u') {
			new_token.type = TokenTypes::u32;
			advance();
			new_token.endSuffixedLiteral(bytes, i, 1);
			return;
		}
		// 64 bit unsigned integer
		else if (byte == 'U') {
			new_token.type = TokenTypes::u64;
			advance();
			new_token.endSuffixedLiteral(bytes, i, 1);
			return;
		}
		// 64 bit signed integer
		else if (byte == 'I') {
			new_token.type = TokenTypes::i64;
			advance();
			new_token.endSuffixedLiteral(bytes, i, 1);
			return;
		}

		// 32 bit float
		else if (byte == 'f') {
			new_token.type = TokenTypes::f32;
			advance();
			new_token.endSuffixedLiteral(bytes, i, 1);
			return;
		}
		// 64 bit float
		else if (byte == 'F') {
			new_token.type = TokenTypes::f64;
			advance();
			new_token.endSuffixedLiteral(bytes, i, 1);
			return;
		}
		else {
			new_token.end(bytes, i);
			return;
		}
	}
}

void Lexer::lexHexadecimal()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::number;
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

void Lexer::lexBinary()
{
	Token& new_token = tokens.emplace_back();
	new_token.type = TokenTypes::number;
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

void Lexer::lexSpacing()
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

void Lexer::lexString()
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

void Lexer::lexVerbatimString()
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

void Lexer::lexSymbol()
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

void Lexer::lexFile(std::vector<uint8_t>& new_bytes, std::string& new_file_path)
{
	file_path = new_file_path;
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

void Lexer::print(bool ignore_spacing)
{
	std::string token_type;

	for (Token& token : tokens) {

		if (token.type != TokenTypes::SPACING) {
			printf("%s (%d, %d): %s = %s \n",
				file_path.c_str(),
				token.line, token.column,
				toStringTokenTypes(token.type).c_str(),
				token.value.c_str());
		}
		else if (ignore_spacing == false) {
			printf("%s (%d, %d): %s \n",
				file_path.c_str(),
				token.line, token.column,
				toStringTokenTypes(token.type).c_str());
		}
	}
}