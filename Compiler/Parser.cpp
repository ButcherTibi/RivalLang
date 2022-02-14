
// Header
#include "Parser.hpp"


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
	uint32_t balance = 0;

	while (i < lexer.tokens.size()) {

		Token& token = getToken(i);

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

bool Parser::parseType(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_type)
{
	auto* type = addNode<AST_Type>(parent_node_index, r_type);

	if (skipToIdentifierToken(i)) {

		type->name = getToken(i);
		type->start_pos = type->name;
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

					type = getNode<AST_Type>(r_type);
					type->end_pos = getToken(i);
					return true;
				}
				else {
					errorUnexpectedToken("while looking for template argument", i);
					return false;
				}
			}
		}
		else {
			type = getNode<AST_Type>(r_type);
			type->end_pos = getToken(i);
			return true;
		}
	}
	else {
		errorUnexpectedToken("unexpected symbol while looking for type name", i);
		return false;
	}

	__debugbreak();
}

void Parser::parseCompositeName(uint32_t& i, std::vector<Token>& r_name)
{
	while (i < lexer.tokens.size()) {

		if (skipToIdentifierToken(i)) {
			r_name.push_back(getToken(i));
			i++;
			
			if (skipToSymbolToken(i, ".")) {
				i++;
				continue;
			}
		}

		return;
	}
}

void Parser::parseModifiers(uint32_t& i, std::vector<Token>& r_modifiers)
{
	while (skipToIdentifierToken(i)) {
		r_modifiers.push_back(getToken(i));
		i++;
	}
}

/* This is the Precedence Climbing Algorithm */
bool Parser::_parseExpression(uint32_t& i, int32_t parent_precedence,
	uint32_t& result)
{
	// parse num (the left hand side of the branch to be created)
	result = 0xFFFF'FFFF;
	{
		uint32_t t;

		if (skipToIdentifierToken(i, t)) {

			i = t;
			throw;
		}
		else if (skipToNumberToken(i, t)) {

			i = t;
			auto num = addNode<AST_Literal>(result);
			num->token = getToken(i);
		}
		else if (skipToStringToken(i, t)) {

			i = t;
			auto str = addNode<AST_Literal>(result);
			str->token = getToken(i);
		}
		else if (skipToSymbolToken(i, "(", t)) {

			i = t;
			i++;
			if (_parseExpression(i, 0, result)) {

				if (skipToSymbolToken(i, ")")) {

					// result/lhs is fine
				}
				else {
					errorUnexpectedToken("while looking for closing ')' in expression", i);
					return false;
				}
			}
			// bad expression in '('
			else {
				return false;
			}
		}
		else {
			throw;
		}

		i++;

		//while (result == 0xFFFF'FFFF) {
		//	
		//	Token* atom_token = &getToken(i);

		//	switch (atom_token->type) {
		//	case TokenTypes::i32:
		//	case TokenTypes::u32:
		//	case TokenTypes::i64:
		//	case TokenTypes::u64:
		//	case TokenTypes::f32:
		//	case TokenTypes::f64:
		//	case TokenTypes::number: {

		//		auto num = addNode<AST_Literal>(result);
		//		num->token = getToken(i);
		//		break;
		//	}

		//	case TokenTypes::STRING: {
		//	
		//		auto str = addNode<AST_Literal>(result);
		//		str->token = getToken(i);
		//		break;
		//	}

		//	case TokenTypes::SYMBOL: {
		//		
		//		if (atom_token->value == "(") {

		//			i++;
		//			if (_parseExpression(i, 0, result)) {

		//				atom_token = &getToken(i);

		//				if (atom_token->isSymbol(")") == false) {

		//					errorUnexpectedToken("while looking for closing ')' in expression", i);
		//					return false;
		//				}
		//			}
		//			else {
		//				return false;
		//			}
		//		}
		//		// end of expression
		//		else {
		//			return true;
		//		}
		//		break;
		//	}

		//	case TokenTypes::SPACING: {
		//		// allowed
		//		break;
		//	}

		//	default: {
		//		__debugbreak();
		//	}
		//	}

		//	i++;
		//}
	}

	while (true) {

		uint32_t r_token_index;
		if (skipToExpressionSymbolToken(i, r_token_index)) {

			i = r_token_index;

			Token& sign_token = getToken(i);

			// current sign precedence
			int32_t precedence;
			{
				if (sign_token.value == "+" || sign_token.value == "-") {
					precedence = 1;
				}
				else if (sign_token.value == "*" || sign_token.value == "/" || sign_token.value == "%") {
					precedence = 2;
				}
				else {
					__debugbreak();
					return false;
				}
			}

			if (precedence >= parent_precedence) {

				i++;
				uint32_t rhs;  // right hand side
				if (_parseExpression(i, precedence, rhs)) {

					// join stuff
					uint32_t sign_node_index;
					{
						auto* binary_op = addNode<AST_BinaryOperator>(sign_node_index);
						binary_op->assign(sign_token);

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
	{
		auto expr = addNode<AST_Expression>(parent_node_index, r_expression);
		expr->start_pos = getToken(i);
	}

	uint32_t expression_root;
	if (_parseExpression(i, 0, expression_root)) {

		if (expression_root == 0xFFFF'FFFF) {
			errorUnexpectedToken("at expression start", i);
			return false;
		}
		else {
			linkParentAndChild(r_expression, expression_root);

			auto expr = getBaseNode(r_expression);
			expr->end_pos = getToken(i);
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
			var_decl->name_token = getToken(i);
			var_decl->start_pos = var_decl->name_token;
		}
		
		i++;

		if (skipToIdentifierToken(i, new_i)) {

			i = new_i;
			uint32_t type;

			if (parseType(r_var_decl_node, i, type)) {

				// default value assignment
				if (skipToSymbolToken(i, "=", new_i)) {

					i = new_i + 1;
					uint32_t default_expr;

					if (parseExpression(r_var_decl_node, i, default_expr)) {

						auto var_decl = getNode<AST_VariableDeclaration>(r_var_decl_node);
						var_decl->type = type;
						var_decl->default_expr = default_expr;
						var_decl->end_pos = getToken(i);
						return true;
					}
					else {
						error("error in default variable value in variable declaration", i);
						return false;
					}
				}

				auto var_decl = getNode<AST_VariableDeclaration>(r_var_decl_node);
				var_decl->type = type;
				var_decl->default_expr = 0xFFFF'FFFF;
				var_decl->end_pos = getToken(i);
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

		assignment->start_pos = getToken(i);

		parseCompositeName(i, assignment->name_tokens);

		if (skipToSymbolToken(i, "=")) {

			i++;

			uint32_t r_expression;
			if (parseExpression(r_assignment_node, i, r_expression)) {

				assignment = getNode<AST_VariableAssignment>(r_assignment_node);
				assignment->end_pos = getToken(i);
				return true;
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
		func_call->start_pos = getToken(i);
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

				func_call = getNode<AST_FunctionCall>(r_func_call_node);
				func_call->end_pos = getToken(i);
				return true;
			}

			parseModifiers(i, func_call->modifiers_tokens);

			func_call = getNode<AST_FunctionCall>(r_func_call_node);
			func_call->end_pos = getToken(i);
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
				i = name_token;
				return parseVariableDeclaration(parent, i, r_statement);
			}
			// variable assignment
			// name =
			else if (skipToSymbolToken(i, "=", new_i)) {
				i = name_token;
				return parseVariableAssignment(parent, i, r_statement);
			}
			// function call
			// name(
			else if (skipToSymbolToken(i, "(", new_i)) {
				i = name_token;
				return parseFunctionCall(parent, i, r_statement);
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
				i = name_token;
				return parseVariableAssignment(parent, i, r_statement);
			}
			// function call
			// name(
			else if (skipToSymbolToken(i, "(", new_i)) {
				i = name_token;
				return parseFunctionCall(parent, i, r_statement);
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

bool Parser::parseStatements(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_statements)
{
	addNode<AST_Statements>(parent_node_index, r_statements);

	uint32_t new_i;
	bool first = true;

	// parse one statement at a time
	while (true) {

		uint32_t statement;
		if (parseStatement(r_statements, i, statement)) {

			if (skipToSymbolToken(i, ";", new_i)) {

				i = new_i + 1;

				if (first == true) {

					auto statements = getBaseNode(r_statements);
					statements->start_pos = getBaseNode(statement)->start_pos;

					first = false;
				}

				if (skipToSymbolToken(i, "}", new_i)) {

					auto statements = getBaseNode(r_statements);
					statements->end_pos = getToken(i);
					return true;
				}
			}
			else {
				errorUnexpectedToken("while looking for statement end or scope end", i);
				return false;
			}
		}
		else {
			return false;
		}
	}
}

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
//	while (i < tokens.size()) {
//
//		Token& token = getToken(i);
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

void Parser::parseFile(std::vector<uint8_t>& file_bytes, std::string file_path)
{	
	lexer.lexFile(file_bytes, file_path);
	// lexer.print(true);

	uint32_t root_node_index;
	addNode<AST_SourceFile>(root_node_index);

	uint32_t i = 0;
	uint32_t child_node_index;

	if (parseStatements(root_node_index, i, child_node_index) == false) {

		printf("\nErrors: \n");

		for (auto& error : errors) {
			printf("(%d, %d) %s \n", error.line, error.column, error.msg.c_str());
		}
	}
}

void Parser::error(std::string msg, uint32_t token_index)
{
	Token& token = getToken(token_index);

	CompilerMessage& new_err = errors.emplace_back();
	new_err.msg = msg;

	new_err.line = token.line;
	new_err.column = token.column;
}

void Parser::errorUnexpectedToken(std::string msg, uint32_t token_index)
{
	std::string message = "unexpected token '" + getToken(token_index).value + "' " + msg;
	error(message, token_index);
}

void Parser::_printTree(uint32_t node_idx, uint32_t depth, PrintAST_TreeSettings& settings)
{
	for (uint32_t i = 0; i < depth; i++) {
		printf("  ");
	}

	AST_BaseNode* node = getBaseNode(node_idx);

	if (settings.show_node_index) {
		printf("%d ", node_idx);
	}

	if (settings.show_source_ranges) {
		printf("{%d %d, %d %d} ",
			node->start_pos.line, node->start_pos.column,
			node->end_pos.line, node->end_pos.column
		);
	}

	printf("%s \n",
		node->toString().c_str());

	for (uint32_t child_node_idx : node->children) {
		_printTree(child_node_idx, depth + 1, settings);
	}
}

void Parser::printTree(PrintAST_TreeSettings settings)
{
	printf("AST Tree: \n");
	_printTree(0, 0, settings);
}

void Parser::printNodes(uint32_t start_index, uint32_t end_index)
{
	if (end_index == 0xFFFF'FFFF) {
		end_index = nodes.size();
	}

	printf("AST Nodes: \n");
	for (uint32_t j = start_index; j < end_index; j++) {

		AST_BaseNode* base_node = getBaseNode(j);
		std::string name = base_node->toString();

		printf("  %d %s parent = %d \n",
			j,
			name.c_str(),
			base_node->parent
		);
	}
}
