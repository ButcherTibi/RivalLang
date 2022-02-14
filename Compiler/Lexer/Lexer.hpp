#pragma once

#include <vector>
#include <string>
#include <inttypes.h>

using namespace std::string_literals;


enum class TokenTypes {
	IDENTIFIER,

	// Integer literals
	i32,
	i64,

	u32,
	u64,

	// Float point literals
	f32,
	f64,

	// hexadecimal, binary literals
	number,

	// string literal
	STRING,

	SYMBOL,

	SPACING
};

std::string toStringTokenTypes(TokenTypes token_type);


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
	void endSuffixedLiteral(std::vector<uint8_t>& bytes, uint32_t end_i, uint32_t suffix_length);

	bool isSpacing();
	bool isNumberLike();
	bool isSymbol();
	bool isSymbol(std::string other);
	bool isExpressionSign();
};


class Lexer {
public:
	std::string file_path;

	// bytes of the file
	std::vector<uint8_t> bytes;
	
	// curent index
	uint32_t i;

	// current line and column
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

	void lexFile(std::vector<uint8_t>& file_bytes, std::string& file_path);


	void print(bool ignore_spacing = false);
};
