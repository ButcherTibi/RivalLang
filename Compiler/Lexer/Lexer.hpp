#pragma once

#include <vector>
#include <string>
#include <cstdint>

using namespace std::string_literals;

struct Token;
class Lexer;

typedef uint32_t TokenIndex;


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

	hexadecimal, // 0xFFAF 08E1	
	binary, // 0b0101 0101

	STRING,
	SYMBOL,
	SPACING
};

std::string toStringTokenTypes(TokenTypes token_type);


struct SourceCodePosition {
	uint32_t line;
	uint32_t column;
};

struct CodeSelection {
	uint32_t file_index;  // in which file does this selection reside

	SourceCodePosition start;
	SourceCodePosition end;

	void operator=(const Token&);
};

struct Token {
	TokenTypes type;

	CodeSelection selection;

	std::string value;

public:
	void start(const Lexer*);
	void end(const Lexer*);
	void assign(const Lexer*, uint32_t start_end, uint32_t end_index);

	bool isSpacing();
	bool isNumberLike();
	bool isSymbol();
	bool isSymbol(std::string other);
	bool isOperator();
};

struct LexerPrintSettings {
	bool ignore_spacing = true;
	bool show_selection = false;
};

class Lexer {
public:
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

	void lexFile(uint32_t file_index, std::vector<uint8_t>& file_bytes);


	void print(LexerPrintSettings settings = LexerPrintSettings());
};
