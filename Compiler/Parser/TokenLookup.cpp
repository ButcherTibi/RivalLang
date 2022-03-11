// Header
#include "./Parser.hpp"


void Parser::advanceToNextToken()
{
	this->token_i += 1;
}

bool Parser::skipSpacing()
{
	TokenIndex i = token_i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.isSpacing() == false) {
			token_i = i;
			return true;
		}

		i++;
	}

	unexpected_idx = token_i;
	return false;
}

bool Parser::skipToIdentifier()
{
	TokenIndex i = token_i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.isSpacing() == false) {

			if (token.type == TokenTypes::IDENTIFIER) {
				token_i = i;
				return true;
			}

			unexpected_idx = i;
			return false;
		}

		i++;
	}

	unexpected_idx = token_i;
	return false;
}

bool Parser::skipToSymbol(std::string symbol)
{
	TokenIndex i = token_i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.isSpacing() == false) {

			if (token.isSymbol(symbol)) {
				token_i = i;
				return true;
			}

			unexpected_idx = i;
			return false;
		}

		i++;
	}

	unexpected_idx = token_i;
	return false;
}

bool Parser::skipToClosingSymbol(std::string starting_symbol, std::string closing_symbol)
{
	uint32_t i = token_i;
	int32_t balance = 0;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.type == TokenTypes::SYMBOL) {

			if (token.value == starting_symbol) {
				balance++;
			}
			else if (token.value == closing_symbol) {
				balance--;

				if (balance <= 0) {
					token_i = i;
					return true;
				}
			}
		}

		i++;
	}

	unexpected_idx = token_i;
	return false;
}
