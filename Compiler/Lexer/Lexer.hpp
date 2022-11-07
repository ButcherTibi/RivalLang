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
	/* @brief In which file does this selection reside */
	uint32_t file_index; 

	SourceCodePosition start;
	SourceCodePosition end;

public:
	CodeSelection() = default;

	void operator=(const Token&);
	void operator=(const std::vector<Token>&);
};


struct Token {
	TokenTypes type;

	/** @brief Where is the token located and what is it's length */
	CodeSelection selection;  

	std::string value;

public:
	/** @brief Mark the start of the token */
	void start(const Lexer*);

	/** @brief Mark the end of the token */
	void end(const Lexer*);

	/** @brief Copy text into token */
	void assign(const Lexer*, uint32_t start_end, uint32_t end_index);

	bool isSpacing();

	bool isNumberLike();
	bool isSignedInteger();

	bool isSymbol();
	bool isSymbol(std::string other);
	bool isOperator();
	bool isOperatorOverload();
};


struct LexerPrintSettings {
	bool ignore_spacing = true;
	bool show_selection = false;
};


class Lexer {
public:
	// bytes of the file
	std::vector<uint8_t> bytes;
	
	// curent index of character in file
	uint32_t i;

	// current line and column
	uint32_t line;
	uint32_t column;

	std::vector<Token> tokens;

private:
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

public:
	void lexFile(uint32_t file_index, std::vector<uint8_t>& file_bytes);


	void print(LexerPrintSettings settings = LexerPrintSettings());
};
