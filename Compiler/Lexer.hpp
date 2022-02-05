#pragma once

#include "Toolbox\utf_string.hpp"


enum class TokenTypes {
	IDENTIFIER,

	// Literals
	NUMBER,
	STRING,

	SYMBOL,

	SPACING
};

struct Token {
	TokenTypes type;

	uint32_t start;
	uint32_t length;

	uint32_t line;
	uint32_t column;

	std::string value;

public:
	void end(uint32_t i_to_next_token);
	void end(std::vector<uint8_t>& bytes, uint32_t i_to_next_token);

	bool isSpacing();
	bool isSymbol();
	bool isSymbol(std::string other);
	bool isExpressionSign();
};


bool isDigit(uint8_t byte);
bool isSmallASCII_Letter(uint8_t byte);
bool isBigASCII_Letter(uint8_t byte);

bool isLetter(uint8_t byte);
bool isSpacing(uint8_t byte);


class Lexer {
public:
	std::string file_path;
	std::vector<uint8_t> bytes;
	uint32_t i;

	uint32_t line;
	uint32_t column;

	std::vector<Token> tokens;

public:

	void advance();

	bool skipToSymbolNoAdvance(uint32_t& index, char symbol);


	// some_letters_with_1234
	void lexIdentifier();

	// float point or integer
	// 10 000 000.890
	void lexNumber();

	// hex notation 0xF1F2 F3F5
	void lexHexadecimal();

	// binary notation 0b0011 1100
	void lexBinary();

	// includes newline and tab
	void lexSpacing();

	// C++ style strings with merging
	void lexString();

	// C# verbatim strings
	void lexVerbatimString();

	// everything, must put this last
	void lexSymbol();

	void lexFile(std::vector<uint8_t>&& file_bytes, std::string& file_path);


	void print(bool ignore_spacing = false);
};
