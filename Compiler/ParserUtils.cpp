
// Header
#include "Parser.hpp"

using namespace std::string_literals;


void SourceCodePosition::operator=(const Token& token)
{
	this->line = token.line;
	this->column = token.column;
}

Token& Parser::getToken(uint32_t token_index)
{
	return lexer.tokens[token_index];
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

	for (Token& name : name_tokens) {

		if (&name != &name_tokens.back()) {
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

	for (Token& name : name_tokens) {

		if (&name != &name_tokens.back()) {
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
	std::string str = std::string("Variable Declaration name = ") + name_token.value;

	return str;
}

std::string AST_FunctionCall::toString()
{
	std::string str = std::string("Function Call name = ");

	for (Token& name : name_tokens) {

		if (&name != &name_tokens.back()) {
			str.append(name.value + std::string("."));
		}
		else {
			str.append(name.value);
		}
	}

	return str;
}

AST_BaseNode* Parser::getBaseNode(uint32_t node_idx)
{
	AST_Node& node = nodes[node_idx];;

	// Code Spliting
	if (std::holds_alternative<AST_SourceFile>(node)) {
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
	else if (std::holds_alternative<AST_FunctionDefinition>(node)) {
		return std::get_if<AST_FunctionDefinition>(&node);
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

void Parser::linkParentAndChild(uint32_t parent_node_index, uint32_t child_node_index)
{
	AST_BaseNode* parent = getBaseNode(parent_node_index);
	parent->children.push_back(child_node_index);

	AST_BaseNode* child = getBaseNode(child_node_index);
	child->parent = parent_node_index;
}
