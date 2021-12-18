
// Header
#include "Parser.hpp"

#include "Toolbox\utf_string.hpp"


std::string AST_BaseNode::toString(std::vector<Token>&)
{
	std::string name;
	switch (ast_node_type) {
	case AST_NodeTypes::FILE:
		return "FILE";
	case AST_NodeTypes::TYPE:
		return "TYPE";
	case AST_NodeTypes::VARIABLE_DECLARATION:
		return "VARIABLE_DECLARATION";
	case AST_NodeTypes::STATEMENTS:
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
	std::string str = std::string("Variable Decl name = ");

	for (uint32_t name_token : name_tokens) {
		str.append(std::string(" ") + tokens[name_token].value);
	}

	if (keyword_tokens.size()) {
		str.append(" keywords =");

		for (uint32_t keyword : keyword_tokens) {
			str.append(std::string(" ") + tokens[keyword].value);
		}
	}
	return str;
}

AST_BaseNode* Parser::getBaseNode(uint32_t node_idx)
{
	AST_Node& node = nodes[node_idx];;

	if (std::holds_alternative<AST_BaseNode>(node)) {
		return std::get_if<AST_BaseNode>(&node);
	}
	else if (std::holds_alternative<AST_Type>(node)) {
		return std::get_if<AST_Type>(&node);
	}
	else if (std::holds_alternative<AST_VariableDeclaration>(node)) {
		return std::get_if<AST_VariableDeclaration>(&node);
	}
	else if (std::holds_alternative<AST_BaseNode>(node)) {
		return std::get_if<AST_BaseNode>(&node);
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
	AST_BaseNode* parent = getBaseNode(parent_node_index);
	parent->children.push_back(child_node_index);

	AST_BaseNode* child = getBaseNode(parent_node_index);
	child->parent = parent_node_index;
}

bool Parser::seekToSymbolToken(uint32_t& i, std::string symbol_token,
	std::vector<std::string> not_allowed_symbols, bool allow_identifier)
{
	uint32_t start = i;

	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

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

	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

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

bool Parser::skipToExpressionSymbolToken(uint32_t i, uint32_t& r_token_index)
{
	uint32_t start = i;

	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

		if (token.type == TokenTypes::SYMBOL) {

			if (token.value == "+" ||
				token.value == "-" ||
				token.value == "*" ||
				token.value == "/" ||
				token.value == "%")
			{
				return true;
			}
		}
		else if (token.type == TokenTypes::SPACING) {
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

bool Parser::skipToNumberToken(uint32_t i, uint32_t& r_token_index)
{
	uint32_t start = i;

	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

		if (token.type == TokenTypes::NUMBER) {
			return true;
		}
		else if (token.type == TokenTypes::SPACING) {
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

bool Parser::skipToStringToken(uint32_t i, uint32_t& r_token_index)
{
	uint32_t start = i;

	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

		if (token.type == TokenTypes::STRING) {
			return true;
		}
		else if (token.type == TokenTypes::SPACING) {
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

bool Parser::skipToClosingSymbolToken(uint32_t& i,
	std::string start_symbol_token, std::string end_symbol_token)
{
	uint32_t start = i;
	uint32_t balance = 0;

	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

		if (token.type == TokenTypes::SYMBOL) {

			if (token.value == start_symbol_token) {
				balance++;
			}
			else if (token.value == end_symbol_token) {
				balance--;

				if (balance == 0) {
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

	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

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

bool Parser::skipToIdentifierToken(uint32_t& i)
{
	uint32_t start = i;

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

	i = start;
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
//
//void Parser::parseVariableDeclaration(uint32_t parent_node_index, uint32_t& i,
//	uint32_t& r_node_index)
//{
//	auto* new_node = addNode<AST_VariableDeclaration>(r_node_index);
//	new_node->ast_node_type = AST_NodeTypes::VARIABLE_DECLARATION;
//
//	linkParentAndChild(parent_node_index, r_node_index);
//
//	enum class Mode {
//		NAME,
//		TYPE,
//		KEYWORD,
//		DEFAULT_VALUE
//	};
//
//	Mode stage = Mode::NAME;
//
//	while (true) {
//
//		Token& token = (*tokens)[i];
//
//		switch (stage) {
//		case Mode::NAME: {
//			if (token.type == TokenTypes::IDENTIFIER) {
//				new_node->name_tokens.push_back();
//				stage = Mode::TYPE;
//			}
//			else if (token.type == TokenTypes::SYMBOL) {
//				throw CompilerErrorException("unexpected symbol while parsing variable declaration name",
//					token.line, token.column
//				);
//			}
//			break;
//		}
//		case Mode::TYPE: {
//
//			uint32_t child_node_index;
//			parseType(r_node_index, i, child_node_index);
//			i--;
//
//			new_node = std::get_if<AST_VariableDeclaration>(&nodes[r_node_index]);
//			//new_node->type_node = child_node_index;
//
//			stage = Mode::KEYWORD;
//			break;
//		}
//		case Mode::KEYWORD: {
//			if (token.type == TokenTypes::IDENTIFIER) {
//				new_node->keyword_tokens.push_back(i);
//			}
//			else if (token.type == TokenTypes::SYMBOL) {
//
//				if (token.value == "=") {
//					
//					// parse expresion
//					__debugbreak();
//				}
//				// end of variable declaration
//				else {
//					return;
//				}
//			}
//			break;
//		}
//		}
//
//		i++;
//	}
//
//	__debugbreak();
//}

void Parser::parseName(uint32_t i,
	std::vector<uint32_t>& r_name, uint32_t& r_token_end)
{
	while (skipToSymbolToken(i, ".", r_token_end)) {

		i = r_token_end + 1;

		if (skipToIdentifierToken(i, r_token_end)) {
			r_name.push_back(i);
		}
		else {
			return;
		}

		i = r_token_end + 1;
	}
}

bool Parser::parseExpression(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_node_index)
{
	{
		auto new_expression = addNode<AST_BaseNode>(r_node_index);
		new_expression->ast_node_type = AST_NodeTypes::EXPRESSION;

		linkParentAndChild(parent_node_index, r_node_index);
	}

	bool is_start = true;

	while (true) {

		std::vector<uint32_t> name_parts;

		uint32_t r_token_index;

		if (skipToIdentifierToken(i, r_token_index)) {

			name_parts.push_back(r_token_index);
			i = r_token_index + 1;

			parseName(i, name_parts, i);
			
			if (skipToIdentifierToken(i, r_token_index)) {
				__debugbreak();
			}
		}
		// +, -, *, /, %
		if (skipToExpressionSymbolToken(i, r_token_index)) {

			i = r_token_index;

			// unary operators can only be used at the start of an expression
			if (is_start) {

				Token& token = (*tokens)[i];

				if (token.value == "+") {

				}
				else if (token.value == "-") {

				}
				else {
					// @HERE
					std::string str = "binary token '" + token.value + "' is used as unary token in expression";
					error(str, i);
					return false;
				}
			}
			// binary operators
			else {

			}
		}
		else if (skipToNumberToken(i, r_token_index)) {
			__debugbreak();
		}
		else if (skipToStringToken(i, r_token_index)) {
			__debugbreak();
		}
		else {
			// let parent scope decide if invalid
			return true;
		}

		is_start = false;
		i++;
	}

	return false;
}

bool Parser::parseStatements(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_node_index)
{
	{
		auto* new_node = addNode<AST_BaseNode>(r_node_index);
		new_node->ast_node_type = AST_NodeTypes::STATEMENTS;

		linkParentAndChild(parent_node_index, r_node_index);
	}

	// parse one statement at a time
	while (true) {

		// std::vector<uint32_t> units;
		std::vector<uint32_t> name_parts;

		uint32_t r_token_index;

		if (skipToIdentifierToken(i, r_token_index)) {

			// units.push_back(i);
			name_parts.push_back(r_token_index);
			i = r_token_index + 1;

			parseName(i, name_parts, i);

			// variable declaration
			if (skipToIdentifierToken(i, r_token_index)) {

				uint32_t new_node_index;
				auto* new_node = addNode<AST_VariableDeclaration>(new_node_index);
				new_node->ast_node_type = AST_NodeTypes::VARIABLE_DECLARATION;
				new_node->name_tokens = name_parts;
				new_node->type_token = i;

				i = r_token_index + 1;

				// modifier
				if (skipToIdentifierToken(i, r_token_index)) {

					new_node->keyword_tokens.push_back(i);
					i = r_token_index + 1;

					// more modifiers
					while (skipToIdentifierToken(i, r_token_index)) {

						new_node->keyword_tokens.push_back(i);
						i = r_token_index + 1;
					}

					// default value assignment
					if (skipToSymbolToken(i, "=", r_token_index)) {
						__debugbreak();
					}
					// default value from constructor
					else if (skipToSymbolToken(i, ";", r_token_index)) {
						__debugbreak();
					}
					else {
						throw CompilerErrorException(
							"unexpected token after variable declaration modifier",
							(*tokens)[i].line, (*tokens)[i].column
						);
					}
				}
				// template param
				else if (skipToSymbolToken(i, "<", r_token_index)) {
					__debugbreak();
				}
				// default value assignment
				else if (skipToSymbolToken(i, "=", r_token_index)) {

					i = r_token_index + 1;
					uint32_t expres_node_idx;

					if (parseExpression(new_node_index, i, expres_node_idx)) {

						if (skipToSymbolToken(i, ";")) {
							// end of statement
						}
						else {
							throw CompilerErrorException(
								"unexpected token after default value of variable",
								(*tokens)[i].line, (*tokens)[i].column
							);
						}
					}
					else {
						return false;
					}
				}
				// default value from constructor
				else if (skipToSymbolToken(i, ";", r_token_index)) {
					// end fo statement
				}
				else {
					throw CompilerErrorException(
						"unexpected token after variable declaration type",
						(*tokens)[i].line, (*tokens)[i].column
					);
				}
			}
			// template params of class
			else if (skipToSymbolToken(i, "<", r_token_index)) {

			}
		}

		i++;

		return true;
	}
}

void Parser::parseFunctionImplementation(uint32_t parent_node_index, uint32_t& i,
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
					//parseVariableDeclaration(r_node_index, i, variable_decl_node);

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
			parseStatements(r_node_index, i, scope_node);
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

bool Parser::isType(uint32_t& i)
{
	if (skipToIdentifierToken(i)) {

		i++;

		// type has template params
		if (skipToSymbolToken(i, "<")) {

			while (true) {

				if (skipToSymbolToken(i, "<")) {
					if (skipToClosingSymbolToken(i, "<", ">") == false) {
						// invalid syntax in template param
						return false;
					};
				}
				else {
					return true;
				}

				i++;
			}
		}

		return true;
	}

	return false;
}

bool Parser::isVariableDeclaration(uint32_t i)
{
	// variable name
	if (skipToIdentifierToken(i)) {
		i++;

		// variable type
		if (isType(i)) {

			if (skipToSymbolToken(i, "<") ||  // var_name template_type<
				skipToSymbolToken(i, "=") ||  // var_name type = 
				skipToSymbolToken(i, ";") ||  // var_name type;
				skipToSymbolToken(i, ",") ||  // var_name type,
				skipToIdentifierToken(i))     // var_name type keyword
			{
				return true;
			}
		}
	}
	
	return false;
}

bool Parser::isFunctionCall(uint32_t i)
{
	// declaration scope
	// - execution scope
	//   - expression scope
	// 
	// - expression scope


	if (skipToIdentifierToken(i)) {
		i++;


		if (skipToSymbolToken(i, "(")) {
			
			i++;

			uint32_t parenthesis_count = 1;

			auto is_close_parenthesis_found = [&]() -> bool {
				while (i < (*tokens).size()) {

					Token& token = (*tokens)[i];

					if (token.type == TokenTypes::SYMBOL) {

						if (token.value == "(") {
							parenthesis_count++;
						}
						else if (token.value == ")") {
							parenthesis_count--;

							if (parenthesis_count == 0) {
								return true;
							}
						}
					}

					i++;
				}

				return false;
			};

			if (is_close_parenthesis_found()) {
				
				i++;

				if (seekToSymbolToken(i, "{")) {
					// this is function implementation
					return false;
				}
				else {
					return true;
				}
			}
		}
	}

	return false;
}

bool Parser::isFunctionImplementation(uint32_t i)
{
	enum class Stages {
		NAME,
		PARAMS_OPEN,
		PARAMS_CLOSE,
		BODY_BEGIN
	};
	auto stage = Stages::NAME;

	while (i < (*tokens).size()) {

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

	return false;
}

void Parser::begin(FileToLex& file_to_lex)
{
	this->tokens = &file_to_lex.tokens;
	
	uint32_t root_node_index;
	auto* root = addNode<AST_BaseNode>(root_node_index);
	root->ast_node_type = AST_NodeTypes::FILE;

	uint32_t i = 0;
	uint32_t child_node_index;

	try {
		parseStatements(root_node_index, i, child_node_index);
	}
	catch (CompilerErrorException& e) {
		printf("Parse error at line = %d column = %d \n",
			e.line, e.column
		);

		printf("%s \n", e.msg.c_str());
	}
}

void Parser::error(std::string msg, uint32_t token_index)
{
	Token& token = (*tokens)[token_index];

	CompilerError& new_err = errors.emplace_back();
	new_err.msg = msg;
	new_err.line = token.line;
	new_err.column = token.column;
}

void Parser::_print(uint32_t node_idx, uint32_t depth)
{
	for (uint32_t i = 0; i < depth; i++) {
		printf("  ");
	}

	AST_BaseNode* node = getBaseNode(node_idx);

	printf("%d %s \n", node_idx, node->toString(*tokens).c_str());

	for (uint32_t child_node_idx : node->children) {
		_print(child_node_idx, depth + 1);
	}
}

void Parser::print()
{
	_print(0, 0);
}
