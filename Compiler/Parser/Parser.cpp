
// Header
#include "Parser.hpp"


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

			std::vector<Token> adress;
			parseCompositeName(i, adress);

			/*if (skipToSymbolToken(i, "(")) {

				parseFunctionCall(0, t)
			}*/

			__debugbreak();
		}
		else if (skipToNumberToken(i, t)) {

			i = t;
			auto num = addNode<AST_Literal>(result);
			num->token = getToken(i);
			num->selection = num->token;
		}
		else if (skipToStringToken(i, t)) {

			i = t;
			auto str = addNode<AST_Literal>(result);
			str->token = getToken(i);
			str->selection = str->token;
		}
		else if (skipToSymbolToken(i, "(", t)) {

			i = t + 1;
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
			__debugbreak();
		}

		i++;
	}

	while (true) {

		uint32_t r_sign;
		if (skipToExpressionSymbolToken(i, r_sign)) {

			i = r_sign;

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
		expr->setStart(getToken(i));
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
			expr->setEnd(getToken(i));
			return true;
		}
	}
	else {
		return false;
	}
}

bool Parser::parseVariableDeclaration(AST_NodeIndex parent_node_index, uint32_t& i,
	AST_NodeIndex& r_var_decl_node)
{
	uint32_t new_i;

	if (skipToIdentifierToken(i, new_i)) {

		i = new_i;
		{
			auto* var_decl = addNode<AST_VariableDeclaration>(parent_node_index, r_var_decl_node);
			var_decl->name = getToken(i);
			var_decl->setStart(var_decl->name);
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
						var_decl->setEnd(getToken(i));
						return true;
					}
					else {
						pushError("Error in default variable value in variable declaration", i);
						return false;
					}
				}

				auto var_decl = getNode<AST_VariableDeclaration>(r_var_decl_node);
				var_decl->type = type;
				var_decl->default_expr = 0xFFFF'FFFF;
				var_decl->setEnd(getToken(i));
				return true;
			}
			else {
				pushError("Error in variable type in variable declaration", i);
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

		assignment->setStart(getToken(i));

		parseCompositeName(i, assignment->address);

		if (skipToSymbolToken(i, "=")) {

			i++;

			uint32_t r_expression;
			if (parseExpression(r_assignment_node, i, r_expression)) {

				assignment = getNode<AST_VariableAssignment>(r_assignment_node);
				assignment->setEnd(getToken(i));
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

bool Parser::parseFunctionImplementation(AST_NodeIndex parent_node_index, uint32_t& i,
	AST_NodeIndex& r_func_impl)
{
	uint32_t new_i;

	if (skipToIdentifierToken(i, new_i)) {

		i = new_i;

		auto* func_impl = addNode<AST_FunctionImplementation>(parent_node_index, r_func_impl);
		func_impl->setStart(getToken(i));

		parseCompositeName(i, func_impl->name);

		if (skipToSymbolToken(i, "(", new_i)) {

			i = new_i + 1;

			// function has NO parameters
			if (skipToSymbolToken(i, ")", new_i)) {

				i = new_i + 1;
			}
			// function has parameters
			else {
				while (true) {

					AST_NodeIndex var_decl_node;

					if (parseVariableDeclaration(r_func_impl, i, var_decl_node)) {

						func_impl = getNode<AST_FunctionImplementation>(r_func_impl);
						func_impl->params.push_back(var_decl_node);

						if (skipToSymbolToken(i, ",")) {
							i++;
						}
						else if (skipToSymbolToken(i, ")")) {
							i++;
							break;
						}
						else {
							errorUnexpectedToken("while looking for function parameter", i);
							return false;
						}
					}
					else {
						return false;
					}
				}
			}

			// function HAS a return type
			if (skipToSymbolToken(i, "{", new_i) == false) {

				uint32_t return_type;
				if (parseType(r_func_impl, i, return_type)) {

					func_impl = getNode<AST_FunctionImplementation>(r_func_impl);
					func_impl->returns = return_type;
				}
				else {
					return false;
				}
			}
			else {
				i = new_i;

				func_impl = getNode<AST_FunctionImplementation>(r_func_impl);
				func_impl->returns = 0xFFFF'FFFF;
			}

			// function body
			AST_NodeIndex ast_statements_idx;
			if (parseStatements(r_func_impl, i, ast_statements_idx)) {

				func_impl = getNode<AST_FunctionImplementation>(r_func_impl);
				func_impl->statements = ast_statements_idx;

				auto* ast_statements = getNode<AST_Statements>(ast_statements_idx);
				func_impl->selection.end = ast_statements->selection.end;
				return true;
			}
			else {
				return false;
			}
		}
		else {
			errorUnexpectedToken("while looking for function parameters", new_i);
			return false;
		}
	}
	else {
		errorUnexpectedToken("while looking for function name", new_i);
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
		func_call->setStart(getToken(i));
		parseCompositeName(i, func_call->address);

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
				func_call->setEnd(getToken(i));
				return true;
			}

			parseModifiers(i, func_call->modifiers_tokens);

			func_call = getNode<AST_FunctionCall>(r_func_call_node);
			func_call->setEnd(getToken(i));
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

bool Parser::parseStatement(AST_NodeIndex parent, uint32_t& i,
	AST_NodeIndex& r_statement)
{
	uint32_t new_i;

	auto check_for_statement_end = [&]() -> bool {

		if (skipToSymbolToken(i, ";", new_i)) {
			i = new_i + 1;
			return true;
		}
		else {
			errorUnexpectedToken("while looking for statement end", i);
			return false;
		}
	};

	if (skipToIdentifierToken(i, new_i)) {

		i = new_i;
		uint32_t name_token = i;

		if (isSimpleName(i)) {

			i++;

			// variable declaration:
			// simple_name type_identifier
			if (skipToIdentifierToken(i, new_i)) {

				i = name_token;

				if (parseVariableDeclaration(parent, i, r_statement)) {
					return check_for_statement_end();
				}
				else {
					return false;
				}
			}
			// variable assignment
			// name =
			else if (skipToSymbolToken(i, "=", new_i)) {

				i = name_token;

				if (parseVariableAssignment(parent, i, r_statement)) {
					return check_for_statement_end();
				}
				else {
					return false;
				}
			}
			// function call
			// name(
			else if (skipToSymbolToken(i, "(", new_i)) {

				i = name_token;

				if (parseFunctionCall(parent, i, r_statement)) {
					return check_for_statement_end();
				}
				else {
					return false;
				}
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

				if (parseVariableAssignment(parent, i, r_statement)) {
					return check_for_statement_end();
				}
				else {
					return false;
				}
			}
			// function call
			// name(
			else if (skipToSymbolToken(i, "(", new_i)) {

				i = name_token;

				if (parseFunctionCall(parent, i, r_statement)) {
					return check_for_statement_end();
				}
				else {
					return false;
				}
			}
			else {
				errorUnexpectedToken("after identifier in statement", new_i);
				return false;
			}
		}
	}
	else {
		pushError("Unrecognized statement", i);
		return false;
	}
}

bool Parser::parseStatements(AST_NodeIndex ast_parent_idx, uint32_t& i,
	AST_NodeIndex& r_statements)
{
	addNode<AST_Statements>(ast_parent_idx, r_statements);

	uint32_t new_i;

	if (skipToSymbolToken(i, "{", new_i)) {

		{
			auto statements = getNode<AST_Statements>(r_statements);
			statements->setStart(getToken(new_i));
		}

		i = new_i + 1;

		while (true) {

			if (skipToSymbolToken(i, "}", new_i)) {

				i = new_i;

				auto statements = getNode<AST_Statements>(r_statements);
				statements->setEnd(getToken(i));

				i++;
				return true;
			}

			uint32_t statement;
			if (parseStatement(r_statements, i, statement) == false) {
				return false;
			}
		}
	}
	else {
		errorUnexpectedToken("while looking for scope start symbol '{'", i);
		return false;
	}

	__debugbreak();
}

bool Parser::parseType(uint32_t parent_node_index, uint32_t& i,
	uint32_t& r_type)
{
	auto* type = addNode<AST_Type>(parent_node_index, r_type);

	if (skipToIdentifierToken(i)) {

		type->name = getToken(i);
		type->setStart(type->name);
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
					type->setEnd(getToken(i));
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
			type->setEnd(getToken(i));
			return true;
		}
	}
	else {
		errorUnexpectedToken("unexpected symbol while looking for type name", i);
		return false;
	}

	__debugbreak();
}

bool Parser::parseDeclaration(AST_NodeIndex ast_parent,
	AST_NodeIndex& r_declaration)
{
	if (skipToIdentifier()) {

		uint32_t name_token = token_i;

		if (isAtAdress() == false) {

			advanceToNextToken();

			// variable declaration:
			// simple_name type_identifier
			if (skipToIdentifier()) {

				token_i = name_token;

				if (parseVariableDeclaration(ast_parent, token_i, r_declaration)) {

					if (skipToSymbol(";")) {
						advanceToNextToken();
						return true;
					}
					else {
						errorUnexpectedToken("while looking for variable declaration end");
						return false;
					}
				}
				else {
					return false;
				}
			}
			// function declaration/implementation
			else if (skipToSymbol("(")) {

				if (skipToClosingSymbol("(", ")")) {

					advanceToNextToken();

					// skip return type and/or modifiers
					{
						while (token_i < lexer.tokens.size()) {

							Token& token = getToken();

							if (token.type == TokenTypes::IDENTIFIER ||
								token.isSymbol("<") || token.isSymbol(">") ||
								token.type == TokenTypes::SPACING)
							{
								advanceToNextToken();
							}
							else {
								break;
							}
						}
					}

					// function implementation
					// no return type and no modifiere
					// ex: func_name() {
					if (skipToSymbol("{")) {

						token_i = name_token;

						if (parseFunctionImplementation(ast_parent, token_i, r_declaration)) {
							return true;
						}
						else {
							return false;
						}
					}
					// function declaration
					// no return type and no modifiere
					// ex. func_name();
					else if (skipToSymbol(";")) {

						__debugbreak();
					}
					else {
						errorUnexpectedToken(
							"after function declaration/implementation parameter list");
						return false;
					}
				}
				else {
					errorUnexpectedToken(
						"while looking for closing ')' in function declaration/implementation");
					return false;
				}
			}
			else {
				errorUnexpectedToken("after identifier in statement");
				return false;
			}
		}
		else {
			// skipPastCompositeName(new_i);

			__debugbreak();
		}
	}
	else {
		errorUnexpectedToken("while looking for declaration");
		return false;
	}

	return false;
}

bool Parser::parseSourceFile(AST_NodeIndex& r_source_file)
{
	addNode<AST_SourceFile>(0, r_source_file);

	this->token_i = 0;
	this->unexpected_idx = 0;

	while (true) {
		
		AST_NodeIndex declaration;
		if (parseDeclaration(r_source_file, declaration)) {

			// end of code
			if (skipSpacing() == false) {
				return true;
			}
		}
		else {
			return false;
		}
	}

	return true;
}

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

bool Parser::isAtAdress()
{
	Token& token = getToken(token_i + 1);

	if (token.isSpacing()) {
		return false;
	}
	else if (token.isSymbol()) {

		if (token.value == "." || token.value == "<") {
			return true;
		}
		return false;
	}

	return true;
}

void Parser::pushError(std::string error_mesage, TokenIndex token_index)
{
	auto& message = messages.emplace_back();
	message.severity = MessageSeverity::Error;

	auto& row_0 = message.rows.emplace_back();
	row_0.text = error_mesage;
	row_0.selection = getToken(token_index);
}

void Parser::errorUnexpectedToken(std::string error_mesage, Token& unexpected_token)
{
	auto& message = messages.emplace_back();
	message.severity = MessageSeverity::Error;

	auto& row_0 = message.rows.emplace_back();
	row_0.text = std::format("Encountered unexpected token '{}' {}",
		unexpected_token.value, error_mesage);
	row_0.selection = unexpected_token;
}

void Parser::errorUnexpectedToken(std::string error_mesage, TokenIndex unexpected_token)
{
	errorUnexpectedToken(error_mesage, getToken(unexpected_token));
}

void Parser::errorUnexpectedToken(std::string error_mesage)
{
	auto& message = messages.emplace_back();
	message.severity = MessageSeverity::Error;

	Token& unexpected_token = getToken(unexpected_idx);

	auto& row_0 = message.rows.emplace_back();
	row_0.text = std::format(
		"Encountered unexpected token '{}' {}",
		unexpected_token.value,
		error_mesage);
	row_0.selection = unexpected_token;
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

	if (settings.show_code_selections) {
		printf("{%d %d, %d %d} ",
			node->selection.start.line, node->selection.start.column,
			node->selection.end.line, node->selection.end.column
		);
	}

	printf("%s \n",
		node->toString().c_str());

	for (uint32_t child_node_idx : node->children) {
		_printTree(child_node_idx, depth + 1, settings);
	}
}

void Parser::printAST(PrintAST_TreeSettings settings)
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
