
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

bool isIdentifier(uint8_t byte)
{
	return
		isDigit(byte) ||
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
