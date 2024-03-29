﻿
// Header
#include "Parser.hpp"


void Parser::parseAdress(std::vector<Token>& r_adress)
{
	while (token_i < lexer.tokens.size()) {

		if (skipToIdentifier()) {

			r_adress.push_back(getToken());
			advanceToNextToken();

			if (skipToSymbol(".")) {
				advanceToNextToken();
				continue;
			}
		}

		return;
	}
}

void Parser::parseModifiers(uint32_t& i, std::vector<Token>& r_modifiers)
{
	while (skipToIdentifier()) {
		r_modifiers.push_back(getToken(i));
		i++;
	}
}

AST_NodeIndex Parser::_parseExpression(int32_t parent_precedence)
{
	AST_NodeIndex ast_left_value;
	{
		if (skipToIdentifier()) {

			TokenIndex begin_adress = token_i;

			if (isAtAdress() == false) {

				advanceToNextToken();

				// function call
				if (skipToSymbol("(")) {

					token_i = begin_adress;

					ast_left_value = parseFunctionCall(ast_invalid_idx);

					if (ast_left_value == ast_invalid_idx) {
						return ast_invalid_idx;
					}
				}
				// variable
				else {
					token_i = begin_adress;

					ast_left_value = parseVariable(ast_invalid_idx);

					if (ast_left_value == ast_invalid_idx) {
						return ast_invalid_idx;
					}
				}
			}
			else {
				std::vector<Token> adress;
				parseAdress(adress);

				// function call
				if (skipToSymbol("(")) {

					token_i = begin_adress;

					ast_left_value = parseFunctionCall(ast_invalid_idx);

					if (ast_left_value == ast_invalid_idx) {
						return ast_invalid_idx;
					}
				}
				// variable
				else {
					token_i = begin_adress;

					ast_left_value = parseVariable(ast_invalid_idx);

					if (ast_left_value == ast_invalid_idx) {
						return ast_invalid_idx;
					}
				}
			}
		}
		else if (skipToNumberLike()) {

			auto* num = addNode<AST_Literal>(ast_left_value);
			num->token = getToken();
			num->selection = num->token;

			advanceToNextToken();
		}
		else if (skipToString()) {

			auto* str = addNode<AST_Literal>(ast_left_value);
			str->token = getToken();
			str->selection = str->token;

			advanceToNextToken();
		}
		else if (skipToSymbol("(")) {

			advanceToNextToken();

			ast_left_value = _parseExpression(0);
			if (ast_left_value != ast_invalid_idx) {

				if (skipToSymbol(")")) {
					advanceToNextToken();
				}
				else {
					logParseError_UnexpectedToken("while looking for closing ')' in expression");
					return ast_invalid_idx;
				}
			}
			// bad expression inside parentheses
			else {
				return ast_invalid_idx;
			}
		}
		else {
			return ast_invalid_idx;
		}
	}

	while (true) {

		if (skipToOperator()) {

			Token& sign = getToken();

			// current sign precedence
			int32_t precedence;
			{
				if (sign.value == "+" || sign.value == "-") {
					precedence = 1;
				}
				else if (sign.value == "*" || sign.value == "/" || sign.value == "%") {
					precedence = 2;
				}
				else {
					throw;
				}
			}

			if (precedence >= parent_precedence) {

				advanceToNextToken();

				AST_NodeIndex ast_right_value = _parseExpression(precedence);
				if (ast_right_value != ast_invalid_idx) {

					AST_NodeIndex ast_binary_op;
					{
						auto* binary_op = addNode<AST_BinaryOperator>(ast_binary_op);
						binary_op->assign(sign);

						linkParentAndChild(ast_binary_op, ast_left_value);
						linkParentAndChild(ast_binary_op, ast_right_value);
					}

					/*
									 next_operator
									/             \
								   /               \
							now_left                new_right
						   /        \
						  /          \
					  left            right
					*/
					ast_left_value = ast_binary_op;
				}
				else {
					
					return ast_invalid_idx;
				}
			}
			// change of precedence
			else {
				return ast_left_value;
			}
		}
		// end of sub expression
		else {
			return ast_left_value;
		}
	}
}

AST_NodeIndex Parser::parseExpression(AST_NodeIndex ast_parent_idx)
{
	AST_NodeIndex r_expression;
	{
		auto* expr = addNode<AST_Expression>(ast_parent_idx, r_expression);
		expr->setStart(getToken());
	}

	AST_NodeIndex expression_root = _parseExpression(0);
	if (expression_root != ast_invalid_idx) {

		linkParentAndChild(r_expression, expression_root);

		auto expr = getBaseNode(r_expression);
		expr->setEnd(getToken(token_i - 1));

		return r_expression;
	}
	else {
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseVariable(AST_NodeIndex parent_node_index)
{
	AST_NodeIndex var_idx;

	if (skipToIdentifier()) {

		if (isAtAdress()) {

			{
				auto* var = addNode<AST_Variable>(parent_node_index, var_idx);
				var->address.push_back(getToken());
			}

			advanceToNextToken();

			return var_idx;
		}
		else {
			{
				auto* var = addNode<AST_Variable>(parent_node_index, var_idx);
				parseAdress(var->address);
			}

			return var_idx;
		}
	}
	else {
		logParseError_UnexpectedToken(
			"while looking for variable name in variable usage");
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseVariableDeclaration(AST_NodeIndex parent_node_index)
{
	AST_NodeIndex r_var_decl_idx;

	if (skipToIdentifier()) {

		{
			auto* var_decl = addNode<AST_VariableDeclaration>(parent_node_index, r_var_decl_idx);
			var_decl->name = getToken();
			var_decl->setStart(var_decl->name);
		}
		
		advanceToNextToken();

		if (skipToIdentifier()) {

			AST_NodeIndex type = parseType(r_var_decl_idx);
			if (type != ast_invalid_idx) {

				// default value assignment
				if (skipToSymbol("=")) {

					advanceToNextToken();

					AST_NodeIndex default_expr = parseExpression(r_var_decl_idx);
					if (default_expr != ast_invalid_idx) {

						auto var_decl = getNode<AST_VariableDeclaration>(r_var_decl_idx);
						var_decl->type = type;
						var_decl->default_expr = default_expr;
						var_decl->setEnd(getToken());

						return r_var_decl_idx;
					}
					else {
						logParseError("Error in default variable value in variable declaration", token_i);
						return ast_invalid_idx;
					}
				}

				auto var_decl = getNode<AST_VariableDeclaration>(r_var_decl_idx);
				var_decl->type = type;
				var_decl->default_expr = 0xFFFF'FFFF;
				var_decl->setEnd(getToken());
				return r_var_decl_idx;
			}
			else {
				logParseError("Error in variable type in variable declaration");
				return ast_invalid_idx;
			}
		}
		else {
			logParseError_UnexpectedToken("while looking for variable type in variable declaration");
			return ast_invalid_idx;
		}
	}
	else {
		logParseError_UnexpectedToken(
			"while looking for variable name in variable declaration");
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseVariableAssignment(AST_NodeIndex ast_parent_idx)
{
	AST_NodeIndex r_assignment;

	if (skipToIdentifier()) {

		{
			auto* assignment = addNode<AST_VariableAssignment>(ast_parent_idx, r_assignment);
			assignment->setStart(getToken());

			parseAdress(assignment->address);
		}

		if (skipToSymbol("=")) {

			advanceToNextToken();

			AST_NodeIndex value_expr_idx = parseExpression(r_assignment);
			if (value_expr_idx != ast_invalid_idx) {

				auto* value_expr = getNode<AST_Expression>(value_expr_idx);

				auto* assignment = getNode<AST_VariableAssignment>(r_assignment);
				assignment->selection.end = value_expr->selection.end;

				return r_assignment;
			}
			else {
				return ast_invalid_idx;
			}
		}
		else {
			logParseError_UnexpectedToken("while looking for equals in assignment");
			return ast_invalid_idx;
		}
	}
	else {
		logParseError_UnexpectedToken("while looking for destination variable name in assignment");
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseFunctionImplementation(AST_NodeIndex ast_parent_idx)
{
	AST_NodeIndex r_func_impl;

	if (skipToIdentifier()) {

		{
			auto* func_impl = addNode<AST_FunctionImplementation>(ast_parent_idx, r_func_impl);
			func_impl->setStart(getToken());

			parseAdress(func_impl->name);
		}

		if (skipToSymbol("(")) {

			advanceToNextToken();

			// function has NO parameters
			if (skipToSymbol(")")) {
				advanceToNextToken();
			}
			// function has parameters
			else {
				while (true) {

					AST_NodeIndex var_decl_idx = parseVariableDeclaration(r_func_impl);
					if (var_decl_idx != ast_invalid_idx) {

						auto* func_impl = getNode<AST_FunctionImplementation>(r_func_impl);
						func_impl->params.push_back(var_decl_idx);

						if (skipToSymbol(",")) {
							advanceToNextToken();
						}
						else if (skipToSymbol(")")) {
							advanceToNextToken();
							break;
						}
						else {
							logParseError_UnexpectedToken("while looking for function parameter");
							return ast_invalid_idx;
						}
					}
					else {
						return ast_invalid_idx;
					}
				}
			}

			// function HAS a return type
			if (skipToSymbol("{") == false) {

				AST_NodeIndex return_type = parseType(r_func_impl);
				if (return_type != ast_invalid_idx) {

					auto* func_impl = getNode<AST_FunctionImplementation>(r_func_impl);
					func_impl->return_type = return_type;
				}
				else {
					return ast_invalid_idx;
				}
			}
			else {
				auto* func_impl = getNode<AST_FunctionImplementation>(r_func_impl);
				func_impl->return_type = 0xFFFF'FFFF;
			}

			// function body
			AST_NodeIndex ast_statements_idx = parseStatements(r_func_impl);
			if (ast_statements_idx != ast_invalid_idx) {

				auto* ast_statements = getNode<AST_Statements>(ast_statements_idx);

				auto* func_impl = getNode<AST_FunctionImplementation>(r_func_impl);
				func_impl->statements = ast_statements_idx;
				func_impl->selection.end = ast_statements->selection.end;

				return r_func_impl;
			}
			else {
				return ast_invalid_idx;
			}
		}
		else {
			logParseError_UnexpectedToken("while looking for function parameters");
			return ast_invalid_idx;
		}
	}
	else {
		logParseError_UnexpectedToken("while looking for function name");
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseFunctionCall(AST_NodeIndex ast_parent_idx)
{
	AST_NodeIndex r_func_call;

	if (skipToIdentifier()) {

		{
			auto* func_call = addNode<AST_FunctionCall>(ast_parent_idx, r_func_call);
			func_call->setStart(getToken());

			parseAdress(func_call->address);
		}

		if (skipToSymbol("(")) {

			advanceToNextToken();
			
			// function call has no arguments
			if (skipToSymbol(")")) {
				
				auto* func_call = getNode<AST_FunctionCall>(r_func_call);
				func_call->setEnd(getToken());

				advanceToNextToken();
				return r_func_call;
			}
			// parse function arguments
			else {
				while (true) {

					AST_NodeIndex argument = parseExpression(r_func_call);
					if (argument != ast_invalid_idx) {

						if (skipToSymbol(",")) {
							advanceToNextToken();
						}
						else if (skipToSymbol(")")) {

							auto* func_call = getNode<AST_FunctionCall>(r_func_call);
							func_call->setEnd(getToken());

							advanceToNextToken();
							return r_func_call;
						}
						else {
							logParseError_UnexpectedToken("while looking for function argument");
							return ast_invalid_idx;
						}
					}
					else {
						return ast_invalid_idx;
					}
				}
			}

			throw;
		}
		else {
			logParseError_UnexpectedToken("while looking for opening '(' in function call");
			return ast_invalid_idx;
		}
	}
	else {
		logParseError_UnexpectedToken("while looking for function name in function call");
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseStatement(AST_NodeIndex parent)
{
	AST_NodeIndex r_statement;

	auto check_for_statement_end = [&]() -> bool {

		if (skipToSymbol(";")) {

			advanceToNextToken();
			return r_statement;
		}
		else {
			logParseError_UnexpectedToken("while looking for statement end");
			return ast_invalid_idx;
		}
	};

	auto check_statement = [&](AST_NodeIndex ast_statement_idx) -> AST_NodeIndex {

		if (ast_statement_idx != ast_invalid_idx) {

			if (skipToSymbol(";")) {

				advanceToNextToken();
				return ast_statement_idx;
			}
			else {
				logParseError_UnexpectedToken("while looking for statement end");
				return ast_invalid_idx;
			}
		}
		else {
			return ast_invalid_idx;
		}
	};

	if (skipToIdentifier()) {

		uint32_t name_token = token_i;

		if (isAtAdress() == false) {

			advanceToNextToken();

			// variable declaration:
			// simple_name type_identifier
			if (skipToIdentifier()) {

				token_i = name_token;
				return check_statement(parseVariableDeclaration(parent));
			}
			// variable assignment
			// name =
			else if (skipToSymbol("=")) {

				token_i = name_token;
				return check_statement(parseVariableAssignment(parent));
			}
			// function call
			// name(
			else if (skipToSymbol("(")) {

				token_i = name_token;
				return check_statement(parseFunctionCall(parent));
			}
			else {
				logParseError_UnexpectedToken("after identifier in statement");
				return ast_invalid_idx;
			}
		}
		else {
			skipPastAdress();

			// variable assignment
			// name =
			if (skipToSymbol("=")) {

				token_i = name_token;
				return check_statement(parseVariableAssignment(parent));
			}
			// function call
			// name(
			else if (skipToSymbol("(")) {

				token_i = name_token;
				return check_statement(parseFunctionCall(parent));
			}
			else {
				logParseError_UnexpectedToken("after identifier in statement");
				return ast_invalid_idx;
			}
		}
	}
	else {
		logParseError("Unrecognized statement");
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseStatements(AST_NodeIndex ast_parent_idx)
{
	AST_NodeIndex r_statements;
	addNode<AST_Statements>(ast_parent_idx, r_statements);

	if (skipToSymbol("{")) {

		{
			auto statements = getNode<AST_Statements>(r_statements);
			statements->setStart(getToken());
		}

		advanceToNextToken();

		while (true) {

			if (skipToSymbol("}")) {

				auto statements = getNode<AST_Statements>(r_statements);
				statements->setEnd(getToken());

				advanceToNextToken();
				return r_statements;
			}

			AST_NodeIndex statement = parseStatement(r_statements);
			if (statement == ast_invalid_idx) {
				return ast_invalid_idx;
			}
		}
	}
	else {
		logParseError_UnexpectedToken("while looking for scope start symbol '{'");
		return ast_invalid_idx;
	}

	throw;
}

AST_NodeIndex Parser::parseOperatorOverload(AST_NodeIndex ast_parent_idx)
{
	AST_NodeIndex r_op_overload;

	if (skipToIdentifier("operator")) {

		{
			Token op_keyword = getToken();
			advanceToNextToken();

			if (getToken().isOperator()) {

				auto* op_overload = addNode<AST_OperatorOverload>(ast_parent_idx, r_op_overload);
				op_overload->op = getToken();
				op_overload->selection.start = op_keyword.selection.start;
				op_overload->selection.end = op_overload->op.selection.end;
			}
			else {
				logParseError_UnexpectedToken("while looking for operator symbol in operator overload");
				return ast_invalid_idx;
			}
		}
		
		advanceToNextToken();

		if (skipToSymbol("(")) {

			advanceToNextToken();

			AST_NodeIndex left_operand_idx = parseVariableDeclaration(r_op_overload);
			if (left_operand_idx != ast_invalid_idx) {

				{
					auto* op_overload = getNode<AST_OperatorOverload>(r_op_overload);
					op_overload->left_operand = left_operand_idx;
				}

				if (skipToSymbol(",")) {
					
					advanceToNextToken();

					AST_NodeIndex right_operand_idx = parseVariableDeclaration(r_op_overload);
					if (right_operand_idx != ast_invalid_idx) {

						{
							auto* op_overload = getNode<AST_OperatorOverload>(r_op_overload);
							op_overload->right_operand = right_operand_idx;
						}
					}
					else {
						return ast_invalid_idx;
					}

					if (skipToSymbol(")")) {

						advanceToNextToken();

						AST_NodeIndex return_type = parseType(r_op_overload);
						if (return_type != ast_invalid_idx) {

							{
								auto* op_overload = getNode<AST_OperatorOverload>(r_op_overload);
								op_overload->return_type = return_type;
							}

							if (skipToSymbol("{") == false) {

								auto* op_overload = getNode<AST_OperatorOverload>(r_op_overload);

								parseModifiers(token_i, op_overload->modifiers);
							}
							
							AST_NodeIndex statements = parseStatements(r_op_overload);
							if (statements != ast_invalid_idx) {

								return r_op_overload;
							}
							else {
								return ast_invalid_idx;
							}
						}
						else {
							return ast_invalid_idx;
						}
					}
					else {
						return ast_invalid_idx;
					}
				}
				else {
					logParseError_UnexpectedToken("while looking for function parameter");
					return ast_invalid_idx;
				}
			}
			else {
				return ast_invalid_idx;
			}
		}
		else {
			logParseError_UnexpectedToken("while looking for operator overload params");
			return ast_invalid_idx;
		}
	}
	else {
		logParseError_UnexpectedToken("while looking for operator overload function name");
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseTypeDeclaration(AST_NodeIndex ast_parent)
{
	AST_NodeIndex r_type_decl;

	if (skipToTypeKeyword()) {

		advanceToNextToken();

		if (skipToIdentifier()) {

			{
				auto* type_decl = addNode<AST_TypeDeclaration>(ast_parent, r_type_decl);
				type_decl->name = getToken();
			}

			advanceToNextToken();

			if (skipToSymbol("{")) {

				advanceToNextToken();

				if (skipToSymbol("}")) {

					advanceToNextToken();
					return r_type_decl;
				}
				else {
					while (true) {
						if (parseDeclaration(r_type_decl) != ast_invalid_idx) {

							if (skipToClosingSymbol("{", "}")) {
								__debugbreak();
							}
							else {
								return ast_invalid_idx;
							}
						}
						else {
							return ast_invalid_idx;
						}
					}
				}
			}
			else {
				logParseError_UnexpectedToken("while looking for type declaration body");
				return ast_invalid_idx;
			}
		}
		else {
			logParseError_UnexpectedToken("while looking for type declaration name");
			return ast_invalid_idx;
		}
	}
	else {
		logParseError_UnexpectedToken("while looking for class/struct keyword");
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseType(AST_NodeIndex ast_parent)
{
	AST_NodeIndex r_type;
	
	if (skipToIdentifier()) {

		{
			auto* type = addNode<AST_Type>(ast_parent, r_type);
			parseAdress(type->address);
			type->setStart(type->address[0]);
		}

		// type has template arguments
		if (skipToSymbol("<")) {

			advanceToNextToken();

			while (true) {

				if (parseType(r_type) == ast_invalid_idx) {
					return ast_invalid_idx;
				}

				if (skipToSymbol(",")) {
					advanceToNextToken();
				}
				else if (skipToSymbol(">")) {

					advanceToNextToken();

					auto* type = getNode<AST_Type>(r_type);
					type->setEnd(getToken());
					return r_type;
				}
				else {
					logParseError_UnexpectedToken("while looking for template argument");
					return ast_invalid_idx;
				}
			}
		}
		else {
			auto* type = getNode<AST_Type>(r_type);
			type->setEnd(getToken());
			return r_type;
		}
	}
	else {
		logParseError_UnexpectedToken("unexpected symbol while looking for type name");
		return ast_invalid_idx;
	}
}

AST_NodeIndex Parser::parseDeclaration(AST_NodeIndex ast_parent)
{
	AST_NodeIndex r_declaration;

	// struct/class
	if (skipToTypeKeyword()) {

		return parseTypeDeclaration(ast_parent);
	}
	// operator overload
	else if (skipToIdentifier("operator")) {

		return parseOperatorOverload(ast_parent);
	}
	// func_name, variable_name
	else if (skipToIdentifier()) {

		uint32_t name_token = token_i;

		if (isAtAdress() == false) {

			advanceToNextToken();

			// variable declaration:
			// simple_name type_identifier
			if (skipToIdentifier()) {

				token_i = name_token;

				r_declaration = parseVariableDeclaration(ast_parent);
				if (r_declaration != ast_invalid_idx) {

					if (skipToSymbol(";")) {
						advanceToNextToken();
						return r_declaration;
					}
					else {
						logParseError_UnexpectedToken("while looking for variable declaration end");
						return ast_invalid_idx;
					}
				}
				else {
					return ast_invalid_idx;
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
						return parseFunctionImplementation(ast_parent);
					}
					// function declaration
					// no return type and no modifiere
					// ex. func_name();
					else if (skipToSymbol(";")) {

						__debugbreak();
					}
					else {
						logParseError_UnexpectedToken(
							"after function declaration/implementation parameter list");
						return ast_invalid_idx;
					}
				}
				else {
					logParseError_UnexpectedToken(
						"while looking for closing ')' in function declaration/implementation");
					return ast_invalid_idx;
				}
			}
			else {
				logParseError_UnexpectedToken("after identifier in statement");
				return ast_invalid_idx;
			}
		}
		else {
			// skipPastCompositeName(new_i);

			__debugbreak();
		}
	}
	else {
		logParseError_UnexpectedToken("while looking for declaration");
		return ast_invalid_idx;
	}

	throw;
}

AST_NodeIndex Parser::parseSourceFile()
{
	AST_NodeIndex r_source_file;
	addNode<AST_SourceFile>(0, r_source_file);

	this->token_i = 0;
	this->unexpected_idx = 0;

	while (true) {
		
		if (parseDeclaration(r_source_file) != 0xFFFF'FFFF) {

			// end of code
			if (skipSpacing() == false) {
				return r_source_file;
			}
		}
		else {
			return ast_invalid_idx;
		}
	}

	throw;
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

void Parser::logParseError(std::string error_mesage, TokenIndex token_index)
{
	auto& message = messages.emplace_back();
	message.severity = MessageSeverity::Error;

	auto& row_0 = message.rows.emplace_back();
	row_0.text = error_mesage;
	row_0.selection = getToken(token_index);
}

void Parser::logParseError(std::string error_mesage)
{
	auto& message = messages.emplace_back();
	message.severity = MessageSeverity::Error;

	auto& row_0 = message.rows.emplace_back();
	row_0.text = error_mesage;
	row_0.selection = getToken(unexpected_idx);
}

void Parser::logParseError_UnexpectedToken(std::string error_mesage)
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
