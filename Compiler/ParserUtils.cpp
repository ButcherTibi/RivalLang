
// Header
#include "Parser.hpp"


Token& Parser::getToken(uint32_t token_index)
{
	return tokens[token_index];
}

std::string AST_SourceFile::toString(std::vector<Token>&)
{
	return "Source File";
};

std::string AST_Type::toString(std::vector<Token>& tokens)
{
	std::string str = std::string("Type ") + std::string(tokens[name_token].value);
	return str;
}

std::string AST_Literal::toString(std::vector<Token>& tokens)
{
	std::string str = std::string("Literal ") + std::string(tokens[token].value);
	return str;
}

std::string AST_Variable::toString(std::vector<Token>& tokens)
{
	std::string str = std::string("Variable name = ");

	for (uint32_t name_token : name_tokens) {

		if (name_token != name_tokens.back()) {
			str.append(tokens[name_token].value + std::string("."));
		}
		else {
			str.append(tokens[name_token].value);
		}
	}

	return str;
}

std::string AST_VariableAssignment::toString(std::vector<Token>& tokens)
{
	std::string str = std::string("Variable Assignment name = ");

	for (uint32_t name_token : name_tokens) {

		if (name_token != name_tokens.back()) {
			str.append(tokens[name_token].value + std::string("."));
		}
		else {
			str.append(tokens[name_token].value);
		}
	}

	return str;
}

std::string AST_VariableDeclaration::toString(std::vector<Token>& tokens)
{
	std::string str = std::string("Variable Declaration name = ") + tokens[name_token].value;

	if (modifiers_tokens.size()) {
		str.append(" modifiers =");

		for (uint32_t modifier : modifiers_tokens) {
			str.append(std::string(" ") + tokens[modifier].value);
		}
	}
	return str;
}

std::string AST_FunctionCall::toString(std::vector<Token>& tokens)
{
	std::string str = std::string("Function Call name = ");

	for (uint32_t name_token : name_tokens) {

		if (name_token != name_tokens.back()) {
			str.append(tokens[name_token].value + std::string("."));
		}
		else {
			str.append(tokens[name_token].value);
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
	else if (std::holds_alternative<AST_OperatorPlusBinary>(node)) {
		return std::get_if<AST_OperatorPlusBinary>(&node);
	}
	else if (std::holds_alternative<AST_OperatorMultiplication>(node)) {
		return std::get_if<AST_OperatorMultiplication>(&node);
	}
	else if (std::holds_alternative<AST_NumericLiteral>(node)) {
		return std::get_if<AST_NumericLiteral>(&node);
	}
	else if (std::holds_alternative<AST_StringLiteral>(node)) {
		return std::get_if<AST_StringLiteral>(&node);
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
