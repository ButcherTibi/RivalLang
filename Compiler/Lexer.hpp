#pragma once

#include "Toolbox\utf_string.hpp"


enum class TokenTypes {
	IDENTIFIER,
	SPACING,
	SYMBOL
};

struct Token {
	TokenTypes type;

	uint32_t start;
	uint32_t length;

	uint32_t line;
	uint32_t column;

	std::string value;

	void end(uint32_t i_to_next_token)
	{
		length = i_to_next_token - start;
	}

	void end(std::vector<uint8_t>& bytes, uint32_t i_to_next_token)
	{
		length = i_to_next_token - start;

		value.resize(length);
		std::memcpy(value.data(), bytes.data() + start, length);
	}
};


bool isDigit(uint8_t byte);
bool isSmallASCII_Letter(uint8_t byte);
bool isBigASCII_Letter(uint8_t byte);

bool isIdentifier(uint8_t byte);
bool isSpacing(uint8_t byte);


class FileToLex {
public:
	std::vector<uint8_t> bytes;
	uint32_t i;

	uint32_t line;
	uint32_t column;

	std::vector<Token> tokens;

public:

	void advance()
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

	void skipInvisible()
	{
		while (i < bytes.size()) {

			uint8_t byte = bytes[i];

			if (byte == ' ' || byte == '\r' || byte == '\n') {
				advance();
			}
			else {
				return;
			}
		}
	}

	void lexIdentifier()
	{
		Token& new_token = tokens.emplace_back();
		new_token.type = TokenTypes::IDENTIFIER;
		new_token.start = i;
		new_token.line = line;
		new_token.column = column;

		while (true) {

			if (isIdentifier(bytes[i]) == false) {
				new_token.end(bytes, i);
				return;
			}

			advance();
		}
	}

	void lexSpacing()
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

	void lexSymbol()
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

	void begin(std::vector<uint8_t>& new_bytes)
	{
		bytes = new_bytes;
		i = 0;

		line = 1;
		column = 1;

		tokens.clear();

		while (i < bytes.size()) {

			uint8_t byte = bytes[i];

			if (isIdentifier(byte)) {
				lexIdentifier();
			}
			else if (isSpacing(byte)) {
				lexSpacing();
			}
			else {
				lexSymbol();
			}
		}
	}

	void print()
	{
		std::string token_type;

		for (Token& token : tokens) {

			switch (token.type) {
			case TokenTypes::IDENTIFIER:
				token_type = "IDENTIFIER";
				break;
			case TokenTypes::SPACING:
				token_type = "SPACING";
				break;
			case TokenTypes::SYMBOL:
				token_type = "SYMBOL";
				break;
			}

			printf("line %d column %d : %s = %s \n",
				token.line, token.column,
				token_type.c_str(), token.value.c_str());
		}
	}
};
