
// Header
#include "Parser.hpp"

using namespace std::string_literals;


void AST_BaseNode::setStart(const Token& token)
{
	selection.start = token.selection.start;
}

void AST_BaseNode::setEnd(const Token& token)
{
	selection.end = token.selection.end;
}

std::string getAdressName(std::vector<Token>& tokens)
{
	std::string result;

	for (Token& name : tokens) {

		if (&name != &tokens.back()) {
			result.append(name.value + std::string("."));
		}
		else {
			result.append(name.value);
		}
	}

	return result;
}

void Parser::init()
{
	AST_Root& root = nodes.emplace_back().emplace<AST_Root>();
	root.parent = 0xFFFF'FFFF;
}

Token& Parser::getToken(uint32_t token_index)
{
	return lexer.tokens[token_index];
}

Token& Parser::getToken()
{
	return lexer.tokens[token_i];
}

std::string AST_Root::toString()
{
	return "AST_Root";
}

std::string AST_SourceFile::toString()
{
	return "Source File";
};

std::string AST_Type::toString()
{
	std::string str = std::string("Type ") + name.value;
	return str;
}

std::string AST_Literal::toString()
{
	return "Literal "s + toStringTokenTypes(token.type) + " = "s + token.value;
}

std::string AST_Variable::toString()
{
	std::string str = std::string("Variable name = ");

	for (Token& name : address) {

		if (&name != &address.back()) {
			str.append(name.value + std::string("."));
		}
		else {
			str.append(name.value);
		}
	}

	return str;
}

std::string AST_VariableAssignment::toString()
{
	std::string str = std::string("Variable Assignment name = ");

	for (Token& name : address) {

		if (&name != &address.back()) {
			str.append(name.value + std::string("."));
		}
		else {
			str.append(name.value);
		}
	}

	return str;
}

std::string AST_VariableDeclaration::toString()
{
	std::string str = std::string("Variable Decl name = ") + name.value;

	return str;
}

std::string AST_FunctionImplementation::toString()
{
	std::string str = std::string("Function Impl name = ");

	for (Token& n : name) {

		if (&n != &name.back()) {
			str.append(n.value + std::string("."));
		}
		else {
			str.append(n.value);
		}
	}

	return str;
}

std::string AST_FunctionCall::toString()
{
	std::string str = std::string("Function Call name = ");

	for (Token& name : address) {

		if (&name != &address.back()) {
			str.append(name.value + std::string("."));
		}
		else {
			str.append(name.value);
		}
	}

	return str;
}

std::string AST_Statements::toString()
{
	return "Statements";
}

AST_BaseNode* Parser::getBaseNode(AST_NodeIndex ast_node_idx)
{
	AST_Node& node = nodes[ast_node_idx];

	if (std::holds_alternative<AST_Root>(node)) {
		return std::get_if<AST_Root>(&node);
	}
	// Code Spliting
	else if (std::holds_alternative<AST_SourceFile>(node)) {
		return std::get_if<AST_SourceFile>(&node);
	}
	// Expression
	else if (std::holds_alternative<AST_Expression>(node)) {
		return std::get_if<AST_Expression>(&node);
	}
	else if (std::holds_alternative<AST_BinaryOperator>(node)) {
		return std::get_if<AST_BinaryOperator>(&node);
	}
	else if (std::holds_alternative<AST_Literal>(node)) {
		return std::get_if<AST_Literal>(&node);
	}
	// Variable
	else if (std::holds_alternative<AST_VariableDeclaration>(node)) {
		return std::get_if<AST_VariableDeclaration>(&node);
	}
	else if (std::holds_alternative<AST_Variable>(node)) {
		return std::get_if<AST_Variable>(&node);
	}
	else if (std::holds_alternative<AST_VariableAssignment>(node)) {
		return std::get_if<AST_VariableAssignment>(&node);
	}
	// Function
	else if (std::holds_alternative<AST_FunctionImplementation>(node)) {
		return std::get_if<AST_FunctionImplementation>(&node);
	}
	else if (std::holds_alternative<AST_FunctionCall>(node)) {
		return std::get_if<AST_FunctionCall>(&node);
	}
	else if (std::holds_alternative<AST_Statements>(node)) {
		return std::get_if<AST_Statements>(&node);
	}
	// Type
	else if (std::holds_alternative<AST_Type>(node)) {
		return std::get_if<AST_Type>(&node);
	}

	__debugbreak();
	throw;
}

CodeSelection Parser::getNodeSelection(AST_NodeIndex ast_node_idx)
{
	return getBaseNode(ast_node_idx)->selection;
}

void Parser::linkParentAndChild(uint32_t parent_node_index, uint32_t child_node_index)
{
	AST_BaseNode* parent = getBaseNode(parent_node_index);
	parent->children.push_back(child_node_index);

	AST_BaseNode* child = getBaseNode(child_node_index);
	child->parent = parent_node_index;
}

bool Parser::seekToSymbolToken(uint32_t& i, std::string symbol_token,
	std::vector<std::string> not_allowed_symbols, bool allow_identifier)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.type == TokenTypes::SYMBOL) {

			for (std::string& not_allowed_symbol : not_allowed_symbols) {

				if (token.value == not_allowed_symbol) {
					return false;
				}
			}

			if (token.value == symbol_token) {
				return true;
			}
		}
		else if (token.type == TokenTypes::IDENTIFIER && allow_identifier == false) {
			return false;
		}

		i++;
	}

	i = start;
	return false;
}

bool Parser::seekToSymbolToken(uint32_t& i, std::string symbol_token)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.type == TokenTypes::SYMBOL) {

			if (token.value == symbol_token) {
				return true;
			}
		}
		else if (token.type == TokenTypes::IDENTIFIER) {
			// allowed
		}
		else {
			return false;
		}

		i++;
	}

	i = start;
	return false;
}

bool Parser::skipSpacing(TokenIndex& i)
{
	TokenIndex start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.isSpacing() == false) {
			return true;
		}

		i++;
	}

	i = start;
	return false;
}

bool Parser::skipToExpressionSymbolToken(uint32_t i, uint32_t& r_token_index)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.isExpressionSign()) {

			r_token_index = i;
			return true;
		}
		else if (token.type == TokenTypes::SPACING) {
			// allowed
		}
		else {
			r_token_index = i;
			return false;
		}

		i++;
	}

	i = start;
	return false;
}

bool Parser::skipToNumberToken(uint32_t i, uint32_t& r_token_index)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.isNumberLike()) {
			r_token_index = i;
			return true;
		}
		else if (token.type == TokenTypes::SPACING) {
			// allowed
		}
		else {
			r_token_index = i;
			return false;
		}

		i++;
	}

	i = start;
	return false;
}

bool Parser::skipToStringToken(uint32_t i, uint32_t& r_token_index)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.type == TokenTypes::STRING) {
			r_token_index = i;
			return true;
		}
		else if (token.type == TokenTypes::SPACING) {
			// allowed
		}
		else {
			r_token_index = i;
			return false;
		}

		i++;
	}

	i = start;
	return false;
}

bool Parser::skipToClosingSymbolToken(uint32_t& i,
	std::string start_symbol_token, std::string end_symbol_token)
{
	uint32_t start = i;
	int32_t balance = 0;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.type == TokenTypes::SYMBOL) {

			if (token.value == start_symbol_token) {
				balance++;
			}
			else if (token.value == end_symbol_token) {
				balance--;

				if (balance <= 0) {
					return true;
				}
			}
		}

		i++;
	}

	i = start;
	return false;
}

bool Parser::skipToSymbolToken(uint32_t& i, std::string target_symbol)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.type == TokenTypes::SYMBOL) {
			if (token.value == target_symbol) {
				return true;
			}
			else {
				return false;
			}
		}
		else if (token.type != TokenTypes::SPACING) {
			return false;
		}

		i++;
	}

	i = start;
	return false;
}

bool Parser::skipToSymbolToken(uint32_t token_index, std::string target_symbol, uint32_t& r_token_index)
{
	r_token_index = token_index;
	return skipToSymbolToken(r_token_index, target_symbol);
}

bool Parser::skipPastIdentifiers(uint32_t& i)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.type == TokenTypes::SPACING || token.type == TokenTypes::IDENTIFIER) {
			i++;
		}
		else {
			return true;
		}
	}

	i = start;
	return false;
}

bool Parser::skipPastCompositeName(uint32_t& i)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		if (skipToIdentifierToken(i)) {

			i++;
			if (skipToSymbolToken(i, ".")) {
				i++;
				continue;
			}
		}

		return true;
	}

	i = start;
	return false;
}

bool Parser::skipToIdentifierToken(uint32_t& i)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.type == TokenTypes::IDENTIFIER) {
			return true;
		}
		else if (token.type != TokenTypes::SPACING) {
			return false;
		}

		i++;
	}

	i = start;
	return false;
}

bool Parser::skipToIdentifierToken(uint32_t token_index, uint32_t& r_token_index)
{
	r_token_index = token_index;
	return skipToIdentifierToken(r_token_index);
}

bool Parser::skipToIdentifierToken(uint32_t& i, std::string target_identifier)
{
	uint32_t start = i;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

		if (token.type == TokenTypes::IDENTIFIER) {
			if (token.value == target_identifier) {
				return true;
			}
			else {
				return false;
			}
		}
		else if (token.type != TokenTypes::SPACING) {
			return false;
		}
	}

	i = start;
	return false;
}

bool Parser::skipToIdentifierToken(uint32_t& token_index, std::string target_identifier, uint32_t& r_token_index)
{
	r_token_index = token_index;
	return skipToIdentifierToken(r_token_index, target_identifier);
}
