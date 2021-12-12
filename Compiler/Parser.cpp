
// Header
#include "Parser.hpp"


std::string AST_NodeBase::toString(std::vector<Token>&)
{
	std::string name;
	switch (ast_node_type) {
	case AST_NodeTypes::ROOT:
		return "ROOT";
	case AST_NodeTypes::TYPE:
		return "TYPE";
	case AST_NodeTypes::VARIABLE_DECLARATION:
		return "VARIABLE_DECLARATION";
	case AST_NodeTypes::SCOPE:
		return "Scope";
	case AST_NodeTypes::FUNCTION_DEFINITION:
		return "FUNCTION_DEFINITION";
	case AST_NodeTypes::FUNCTION_CALL:
		return "FUNCTION_CALL";
	}

	__debugbreak();
	return "";
}

std::string AST_Type::toString(std::vector<Token>& tokens)
{
	std::string str = std::string("Type ") + std::string(tokens[name_token].value);
	return str;
}

std::string AST_VariableDeclaration::toString(std::vector<Token>& tokens)
{
	std::string str = std::string("Variable Decl name = ") + std::string(tokens[name_token].value);

	if (keyword_tokens.size()) {
		str.append(" keywords =");

		for (uint32_t keyword : keyword_tokens) {
			str.append(std::string(" ") + tokens[keyword].value);
		}
	}
	return str;
}

AST_NodeBase* Parser::getBaseNode(uint32_t node_idx)
{
	AST_Node& node = nodes[node_idx];;

	if (std::holds_alternative<AST_FileRoot>(node)) {
		return std::get_if<AST_FileRoot>(&node);
	}
	else if (std::holds_alternative<AST_Type>(node)) {
		return std::get_if<AST_Type>(&node);
	}
	else if (std::holds_alternative<AST_VariableDeclaration>(node)) {
		return std::get_if<AST_VariableDeclaration>(&node);
	}
	else if (std::holds_alternative<AST_Scope>(node)) {
		return std::get_if<AST_Scope>(&node);
	}
	else if (std::holds_alternative<AST_FunctionCall>(node)) {
		return std::get_if<AST_FunctionCall>(&node);
	}
	else if (std::holds_alternative<AST_FunctionDefinition>(node)) {
		return std::get_if<AST_FunctionDefinition>(&node);
	}

	__debugbreak();
	return nullptr;
}

void Parser::linkParentAndChild(uint32_t parent_node_index, uint32_t child_node_index)
{
	AST_NodeBase* parent = getBaseNode(parent_node_index);
	parent->children.push_back(child_node_index);

	AST_NodeBase* child = getBaseNode(parent_node_index);
	child->parent = parent_node_index;
}

uint32_t Parser::seekToSymbolToken(uint32_t i, std::string symbol_token,
	std::vector<std::string> not_allowed_symbols, bool allow_identifier)
{
	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

		if (token.type == TokenTypes::SYMBOL) {

			for (std::string& not_allowed_symbol : not_allowed_symbols) {

				if (token.value == not_allowed_symbol) {
					return 0xFFFF'FFFF;
				}
			}

			if (token.value == symbol_token) {
				return i;
			}
		}
		else if (token.type == TokenTypes::IDENTIFIER && allow_identifier == false) {
			return 0xFFFF'FFFF;
		}

		i++;
	}

	return 0xFFFF'FFFF;
}

bool Parser::skipToSymbolToken(uint32_t& i, std::string target_symbol)
{
	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

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

	i = 0xFFFF'FFFF;
	return false;
}

bool Parser::skipToIdentifierToken(uint32_t& i)
{
	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

		if (token.type == TokenTypes::IDENTIFIER) {
			return true;
		}
		else if (token.type != TokenTypes::SPACING) {
			return false;
		}

		i++;
	}

	i = 0xFFFF'FFFF;
	return false;
}

bool Parser::skipToIdentifierToken(uint32_t& i, std::string target_identifier)
{
	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

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

	i = 0xFFFF'FFFF;
	return false;
}

void Parser::parseType(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_node_index)
{
	{
		auto* new_node = addNode<AST_Type>(r_node_index);
		new_node->ast_node_type = AST_NodeTypes::TYPE;
	}

	linkParentAndChild(parent_node_index, r_node_index);

	if (skipToIdentifierToken(i)) {

		{
			auto* new_node = getNode<AST_Type>(r_node_index);
			new_node->name_token = i;
		}

		i++;

		// type has template arguments
		if (skipToSymbolToken(i, "<")) {

			i++;

			while (true) {

				uint32_t child_node;
				parseType(r_node_index, i, child_node);

				if (skipToSymbolToken(i, ",")) {
					i++;
				}
				else if (skipToSymbolToken(i, ">")) {
					i++;
					return;
				}
				else {
					throw CompilerErrorException(
						"unexpected symbol while looking for template argument",
						(*tokens)[i].line, (*tokens)[i].column
					);
				}
			}
		}
		else {
			return;
		}
	}
	else {
		throw CompilerErrorException(
			"unexpected symbol while looking for type name",
			(*tokens)[i].line, (*tokens)[i].column
		);
	}

	__debugbreak();
}

void Parser::parseVariableDeclaration(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_node_index)
{
	auto* new_node = addNode<AST_VariableDeclaration>(r_node_index);
	new_node->ast_node_type = AST_NodeTypes::VARIABLE_DECLARATION;

	linkParentAndChild(parent_node_index, r_node_index);

	enum class Mode {
		NAME,
		TYPE,
		KEYWORD,
		DEFAULT_VALUE
	};

	Mode stage = Mode::NAME;

	while (true) {

		Token& token = (*tokens)[i];

		switch (stage) {
		case Mode::NAME: {
			if (token.type == TokenTypes::IDENTIFIER) {
				new_node->name_token = i;
				stage = Mode::TYPE;
			}
			else if (token.type == TokenTypes::SYMBOL) {
				throw CompilerErrorException("unexpected symbol while parsing variable declaration name",
					token.line, token.column
				);
			}
			break;
		}
		case Mode::TYPE: {

			uint32_t child_node_index;
			parseType(r_node_index, i, child_node_index);
			i--;

			new_node = std::get_if<AST_VariableDeclaration>(&nodes[r_node_index]);
			new_node->type_node = child_node_index;

			stage = Mode::KEYWORD;
			break;
		}
		case Mode::KEYWORD: {
			if (token.type == TokenTypes::IDENTIFIER) {
				new_node->keyword_tokens.push_back(i);
			}
			else if (token.type == TokenTypes::SYMBOL) {

				if (token.value == "=") {
					
					// parse expresion
					__debugbreak();
				}
				// end of variable declaration
				else {
					return;
				}
			}
			break;
		}
		}

		i++;
	}

	__debugbreak();
}

void Parser::parseScope(uint32_t parent_node_index, uint32_t& token_index,
	uint32_t& r_node_index)
{
	{
		auto* new_node = addNode<AST_Scope>(r_node_index);
		new_node->ast_node_type = AST_NodeTypes::SCOPE;
	}

	linkParentAndChild(parent_node_index, r_node_index);


}

void Parser::parseFunctionDefinition(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_node_index)
{
	{
		auto* new_node = addNode<AST_FunctionDefinition>(r_node_index);
		new_node->ast_node_type = AST_NodeTypes::FUNCTION_DEFINITION;
	}
	
	linkParentAndChild(parent_node_index, r_node_index);

	if (skipToIdentifierToken(i)) {

		{
			auto* new_node = std::get_if<AST_FunctionDefinition>(&nodes[r_node_index]);
			new_node->name_token = i;
		}

		i++;

		if (skipToSymbolToken(i, "(")) {
			
			i++;
			uint32_t token_index = i;

			// function has NO parameters
			if (skipToSymbolToken(token_index, ")")) {

				i = token_index + 1;
			}
			// function has parameters
			else {
				while (true) {

					uint32_t variable_decl_node;
					parseVariableDeclaration(r_node_index, i, variable_decl_node);

					{
						auto* new_node = std::get_if<AST_FunctionDefinition>(&nodes[r_node_index]);
						new_node->param_nodes.push_back(variable_decl_node);
					}


					if (skipToSymbolToken(i, ",")) {
						i++;
					}
					else if (skipToSymbolToken(i, ")")) {
						i++;
						break;
					}
					else {
						throw CompilerErrorException(
							"unexpected token while looking for function parameters",
							(*tokens)[i].line, (*tokens)[i].column
						);
					}
				}
			}

			token_index = i;

			// function has NO return type
			if (skipToSymbolToken(token_index, "{")) {

				i = token_index + 1;

				auto* new_node = std::get_if<AST_FunctionDefinition>(&nodes[r_node_index]);
				new_node->return_node = 0xFFFF'FFFF;
			}
			else {
				uint32_t return_type;
				parseType(r_node_index, i, return_type);

				auto* new_node = std::get_if<AST_FunctionDefinition>(&nodes[r_node_index]);
				new_node->return_node = return_type;
			}
			
			// function body
			uint32_t scope_node;
			parseScope(r_node_index, i, scope_node);
		}
		else {
			throw CompilerErrorException(
				"unexpected token while looking for function parameters",
				(*tokens)[i].line, (*tokens)[i].column
			);
		}
	}
	else {
		throw CompilerErrorException(
			"unexpected token while looking for function name",
			(*tokens)[i].line, (*tokens)[i].column
		);
	}
}

bool Parser::isFunctionDefinition(uint32_t i)
{
	enum class Stages {
		NAME,
		PARAMS_OPEN,
		PARAMS_CLOSE,
		BODY_BEGIN
	};
	auto stage = Stages::NAME;

	while (true) {

		Token& token = (*tokens)[i];

		switch (stage) {
		case Stages::NAME: {
			if (token.type == TokenTypes::IDENTIFIER) {
				stage = Stages::PARAMS_OPEN;
			}
			else if (token.type == TokenTypes::SYMBOL) {
				return false;
			}
			break;
		}
		case Stages::PARAMS_OPEN: {
			if (token.type == TokenTypes::SYMBOL) {

				if (token.value == "(") {
					stage = Stages::PARAMS_CLOSE;
				}
				else {
					return false;
				}
			}
			else if (token.type == TokenTypes::IDENTIFIER) {
				return false;
			}
			break;
		}
		case Stages::PARAMS_CLOSE: {
			if (token.type == TokenTypes::SYMBOL) {

				if (token.value == ")") {
					stage = Stages::BODY_BEGIN;
				}
				else if (token.value == "{" || token.value == "}") {
					return false;
				}
			}
			break;
		}
		case Stages::BODY_BEGIN: {
			if (token.type == TokenTypes::SYMBOL) {

				if (token.value == "{") {
					return true;
				}
				else if (token.value == "}" ||
					token.value == "(" || token.value == ")")
				{
					return false;
				}
			}
			break;
		}
		}

		i++;
	}

	__debugbreak();
	return false;
}

void Parser::begin(FileToLex& file_to_lex)
{
	this->tokens = &file_to_lex.tokens;
	
	uint32_t root_node_index;
	auto* root = addNode<AST_FileRoot>(root_node_index);
	root->ast_node_type = AST_NodeTypes::ROOT;

	try {
		uint32_t i = 0;
		uint32_t child_node_index;

		parseFunctionDefinition(root_node_index, i, child_node_index);
	}
	catch (CompilerErrorException& e) {
		printf("Parse error at line = %d column = %d \n",
			e.line, e.column
		);

		printf("%s \n", e.msg.c_str());
	}
}

void Parser::_print(uint32_t node_idx, uint32_t depth)
{
	for (uint32_t i = 0; i < depth; i++) {
		printf("  ");
	}

	AST_NodeBase* node = getBaseNode(node_idx);

	printf("%d %s \n", node_idx, node->toString(*tokens).c_str());

	for (uint32_t child_node_idx : node->children) {
		_print(child_node_idx, depth + 1);
	}
}

void Parser::print()
{
	_print(0, 0);
}
