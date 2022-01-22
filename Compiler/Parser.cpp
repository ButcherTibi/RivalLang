
// Header
#include "Parser.hpp"

#include "Toolbox\utf_string.hpp"


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

	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

		if (token.type == TokenTypes::NUMBER) {
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

	while (i < tokens->size()) {

		Token& token = (*tokens)[i];

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

bool Parser::skipPastCompositeName(uint32_t& i)
{
	uint32_t start = i;

	while (i < tokens->size()) {

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

bool Parser::parseType(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_type)
{
	auto* type = addNode<AST_Type>(parent_node_index, r_type);

	if (skipToIdentifierToken(i)) {

		type->name_token = i;
		i++;

		// type has template arguments
		if (skipToSymbolToken(i, "<")) {

			i++;

			while (true) {

				uint32_t child_node;
				if (parseType(r_type, i, child_node) == false) {
					return false;
				}

				if (skipToSymbolToken(i, ",")) {
					i++;
				}
				else if (skipToSymbolToken(i, ">")) {
					i++;
					return true;
				}
				else {
					errorUnexpectedToken("while looking for template argument", i);
					return false;
				}
			}
		}
		else {
			return true;
		}
	}
	else {
		errorUnexpectedToken("unexpected symbol while looking for type name", i);
		return false;
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

void Parser::parseCompositeName(uint32_t& i, std::vector<uint32_t>& r_name)
{
	while (i < tokens->size()) {

		if (skipToIdentifierToken(i)) {
			r_name.push_back(i);
			i++;
			
			if (skipToSymbolToken(i, ".")) {
				i++;
				continue;
			}
		}

		return;
	}
}

void Parser::parseModifiers(uint32_t& i, std::vector<uint32_t>& r_modifiers)
{
	while (skipToIdentifierToken(i)) {
		r_modifiers.push_back(i);
		i++;
	}
}

/* This is the Precedence Climbing Algorithm */
bool Parser::parseSubExpression(uint32_t& i, int32_t parent_precedence,
	uint32_t& result)
{
	// parse atom (the left hand side of the branch to be created)
	result = 0xFFFF'FFFF;
	{
		while (result == 0xFFFF'FFFF) {
			
			Token* atom_token = &getToken(i);

			switch (atom_token->type) {
			case TokenTypes::NUMBER: {

				auto atom = addNode<AST_NumericLiteral>(result);
				atom->token = i;
				break;
			}

			case TokenTypes::STRING: {
				
				auto atom = addNode<AST_StringLiteral>(result);
				atom->token = i;
				break;
			}

			case TokenTypes::SYMBOL: {
				
				if (atom_token->value == "(") {

					i++;
					if (parseSubExpression(i, 0, result)) {

						atom_token = &(*tokens)[i];

						if (atom_token->isSymbol(")") == false) {

							errorUnexpectedToken("while looking for closing ')' in expression", i);
							return false;
						}
					}
					else {
						return false;
					}
				}
				// end of expression
				else {
					return true;
				}
				break;
			}

			case TokenTypes::SPACING: {
				// allowed
				break;
			}

			default: {
				__debugbreak();
			}
			}

			i++;
		}
	}

	while (true) {

		uint32_t r_token_index;
		if (skipToExpressionSymbolToken(i, r_token_index)) {

			i = r_token_index;

			Token& sign_token = (*tokens)[i];

			int32_t precedence;

			if (sign_token.value == "+" || sign_token.value == "-") {
				precedence = 1;
			}
			else if (sign_token.value == "*" || sign_token.value == "/" || sign_token.value == "%") {
				precedence = 2;
			}
			else {
				__debugbreak();
			}

			if (precedence >= parent_precedence) {

				i++;
				uint32_t rhs;  // right hand side
				if (parseSubExpression(i, precedence, rhs)) {

					if (rhs == 0xFFFF'FFFF) {
						// end of sub expression
						return true;
					}

					// join stuff
					uint32_t sign_node_index;
					{
						if (sign_token.value == "+") {
							addNode<AST_OperatorPlusBinary>(sign_node_index);
						}
						else if (sign_token.value == "*") {
							addNode<AST_OperatorMultiplication>(sign_node_index);
						}
						else {
							__debugbreak();
						}

						/*
								 sign
								/    \
							   /      \
						  result      right
						  (left)
						*/
						linkParentAndChild(sign_node_index, result);
						linkParentAndChild(sign_node_index, rhs);
					}

					/*
									 next_sign
									/         \
								   /           \
							   sign             new_right
							 (result)
							/        \
						   /          \
					  old_result       right
					  (left)
					*/
					result = sign_node_index;
				}
				// error
				else {
					return false;
				}
			}
			// change of precedence
			else {			
				return true;
			}
		}
		// end of sub expression
		else {
			return true;
		}
	}

	__debugbreak();
	return false;
}

bool Parser::parseExpression(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_expression)
{
	addNode<AST_Expression>(parent_node_index, r_expression);

	uint32_t expression_root;
	if (parseSubExpression(i, 0, expression_root)) {

		if (expression_root == 0xFFFF'FFFF) {
			errorUnexpectedToken("at expression start", i);
			return false;
		}
		else {
			linkParentAndChild(r_expression, expression_root);
			return true;
		}
	}
	else {
		return false;
	}
}

bool Parser::parseVariableDeclaration(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_var_decl_node)
{
	uint32_t new_i;

	if (skipToIdentifierToken(i, new_i)) {

		i = new_i;
		{
			auto* var_decl = addNode<AST_VariableDeclaration>(parent_node_index, r_var_decl_node);
			var_decl->name_token = i;
		}
		
		i++;

		if (skipToIdentifierToken(i, new_i)) {

			i = new_i;
			uint32_t type;

			if (parseType(r_var_decl_node, i, type)) {

				// default value assignment
				if (skipToSymbolToken(i, "=", new_i)) {

					i = new_i + 1;
					uint32_t expres_node_idx;

					if (parseExpression(r_var_decl_node, i, expres_node_idx)) {
						return true;
					}
					else {
						error("error in default variable value in variable declaration", i);
						return false;
					}
				}

				return true;
			}
			else {
				error("error in variable type in variable declaration", i);
				return false;
			}
		}
		else {
			errorUnexpectedToken("while looking for variable type in variable declaration", new_i);
			return false;
		}
	}
	else {
		errorUnexpectedToken("while looking for variable name in variable declaration", new_i);
		return false;
	}
}

bool Parser::parseVariableAssignment(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_assignment_node)
{
	auto assignment = addNode<AST_VariableAssignment>(parent_node_index, r_assignment_node);

	if (skipToIdentifierToken(i)) {

		parseCompositeName(i, assignment->name_tokens);

		if (skipToSymbolToken(i, "=")) {

			i++;

			uint32_t r_expression;
			if (parseExpression(r_assignment_node, i, r_expression)) {

				if (getToken(i).value == ";") {
					i++;
					return true;
				}
				else {
					errorUnexpectedToken("while looking for end of assignment ';'", i);
					return false;
				}
			}
			else {
				return false;
			}
		}
		else {
			errorUnexpectedToken("while looking for equals in assignment", i);
			return false;
		}
	}
	else {
		errorUnexpectedToken("while looking for destination variable name in assignment", i);
		return false;
	}
}

bool Parser::parseFunctionCall(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_func_call_node)
{
	auto func_call = addNode<AST_FunctionCall>(parent_node_index, r_func_call_node);

	uint32_t new_i;

	if (skipToIdentifierToken(i, new_i)) {

		i = new_i;
		parseCompositeName(i, func_call->name_tokens);

		if (skipToSymbolToken(i, "(", new_i)) {

			i = new_i;
			i++;
			
			// function call has no arguments
			if (skipToSymbolToken(i, ")", new_i)) {
				i = new_i;
			}
			// parse function arguments
			else {
				while (true) {

					uint32_t expr;
					if (parseExpression(r_func_call_node, i, expr)) {

						if (skipToSymbolToken(i, ",", new_i)) {

							i = new_i + 1;
						}
						else if (skipToSymbolToken(i, ")", new_i)) {
							i = new_i + 1;
							break;
						}
						else {
							errorUnexpectedToken("while looking for function argument", new_i);
							return false;
						}
					}
					else {
						return false;
					}
				}

				return true;
			}

			parseModifiers(i, func_call->modifiers_tokens);
			return true;
		}
		else {
			errorUnexpectedToken("while looking for opening '(' in function call", new_i);
			return false;
		}
	}
	else {
		errorUnexpectedToken("while looking for function name in function call", new_i);
		return false;
	}
}

bool Parser::parseStatement(uint32_t parent, uint32_t& i,
	uint32_t& r_statement)
{
	uint32_t new_i;

	if (skipToIdentifierToken(i, new_i)) {

		i = new_i;
		uint32_t name_token = i;

		if (isSimpleName(i)) {

			i++;

			// variable declaration:
			// simple_name type_identifier
			if (skipToIdentifierToken(i, new_i)) {
				return parseVariableDeclaration(parent, name_token, r_statement);
			}
			// variable assignment
			// name =
			else if (skipToSymbolToken(i, "=", new_i)) {
				return parseVariableAssignment(parent, name_token, r_statement);
			}
			// function call
			// name(
			else if (skipToSymbolToken(i, "(", new_i)) {
				return parseFunctionCall(parent, name_token, r_statement);
			}
			else {
				errorUnexpectedToken("after identifier in statement", new_i);
				return false;
			}
		}
		else {
			skipPastCompositeName(i);

			// variable assignment
			// name =
			if (skipToSymbolToken(i, "=", new_i)) {
				return parseVariableAssignment(parent, name_token, r_statement);
			}
			// function call
			// name(
			else if (skipToSymbolToken(i, "(", new_i)) {
				return parseFunctionCall(parent, name_token, r_statement);
			}
			else {
				errorUnexpectedToken("after identifier in statement", new_i);
				return false;
			}
		}
	}
	else {
		error("unrecognized statement", i);
		return false;
	}
}

//bool Parser::parseStatements(uint32_t parent_node_index, uint32_t& i,
//	uint32_t& r_statements)
//{
//	{
//		addNode<AST_Statements>(r_statements);
//		linkParentAndChild(parent_node_index, r_statements);
//	}
//
//	// parse one statement at a time
//	while (true) {
//
//		return true;
//	}
//}

bool Parser::parseFunctionImplementation(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_node_index)
{
	{
		addNode<AST_FunctionDefinition>(r_node_index);
		linkParentAndChild(parent_node_index, r_node_index);
	}

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
						errorUnexpectedToken("while looking for function parameters", i);
						return false;
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
			// uint32_t scope_node;
			// parseStatements(r_node_index, i, scope_node);
		}
		else {
			errorUnexpectedToken("while looking for function parameters", i);
			return false;
		}
	}
	else {
		errorUnexpectedToken("while looking for function name", i);
		return false;
	}

	return true;
}

//bool Parser::isFunctionImplementation(uint32_t i)
//{
//	enum class Stages {
//		NAME,
//		PARAMS_OPEN,
//		PARAMS_CLOSE,
//		BODY_BEGIN
//	};
//	auto stage = Stages::NAME;
//
//	while (i < (*tokens).size()) {
//
//		Token& token = (*tokens)[i];
//
//		switch (stage) {
//		case Stages::NAME: {
//			if (token.type == TokenTypes::IDENTIFIER) {
//				stage = Stages::PARAMS_OPEN;
//			}
//			else if (token.type == TokenTypes::SYMBOL) {
//				return false;
//			}
//			break;
//		}
//		case Stages::PARAMS_OPEN: {
//			if (token.type == TokenTypes::SYMBOL) {
//
//				if (token.value == "(") {
//					stage = Stages::PARAMS_CLOSE;
//				}
//				else {
//					return false;
//				}
//			}
//			else if (token.type == TokenTypes::IDENTIFIER) {
//				return false;
//			}
//			break;
//		}
//		case Stages::PARAMS_CLOSE: {
//			if (token.type == TokenTypes::SYMBOL) {
//
//				if (token.value == ")") {
//					stage = Stages::BODY_BEGIN;
//				}
//				else if (token.value == "{" || token.value == "}") {
//					return false;
//				}
//			}
//			break;
//		}
//		case Stages::BODY_BEGIN: {
//			if (token.type == TokenTypes::SYMBOL) {
//
//				if (token.value == "{") {
//					return true;
//				}
//				else if (token.value == "}" ||
//					token.value == "(" || token.value == ")")
//				{
//					return false;
//				}
//			}
//			break;
//		}
//		}
//
//		i++;
//	}
//
//	return false;
//}

bool Parser::isSimpleName(uint32_t token_index)
{
	Token& token = getToken(token_index + 1);

	if (token.isSpacing()) {
		return true;
	}
	else if (token.isSymbol()) {

		if (token.value == "." || token.value == "<") {
			return false;
		}
		return true;
	}

	return false;
}

void Parser::parseSourceFile(FileToLex& file_to_lex)
{
	this->tokens = &file_to_lex.tokens;
	
	uint32_t root_node_index;
	addNode<AST_SourceFile>(root_node_index);

	uint32_t i = 0;
	uint32_t child_node_index;

	parseVariableDeclaration(root_node_index, i, child_node_index);
	// parseStatement(root_node_index, i, child_node_index);

	if (errors.size()) {

		printf("\nErrors: \n");

		for (auto& error : errors) {
			printf("(%d, %d) %s \n", error.line, error.column, error.msg.c_str());
		}
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

void Parser::errorUnexpectedToken(std::string msg, uint32_t token_index)
{
	Token& token = (*tokens)[token_index];

	CompilerError& new_err = errors.emplace_back();
	new_err.msg = "unexpected token '" + token.value + "' " + msg;

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

void Parser::printTree()
{
	printf("AST Tree: \n");
	_print(0, 0);
}

void Parser::printNodes(uint32_t start_index, uint32_t end_index)
{
	if (end_index == 0xFFFF'FFFF) {
		end_index = nodes.size();
	}

	printf("AST Nodes: \n");
	for (uint32_t j = start_index; j < end_index; j++) {

		AST_BaseNode* base_node = getBaseNode(j);
		std::string name = base_node->toString(*tokens);

		printf("  %d %s parent = %d \n",
			j,
			name.c_str(),
			base_node->parent
		);
	}
}
