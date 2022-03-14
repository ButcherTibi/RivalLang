
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
	std::string str = std::string("Type ") + getAdressName(address);
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
