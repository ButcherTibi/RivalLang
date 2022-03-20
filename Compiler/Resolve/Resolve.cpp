
// Header
#include "Resolve.hpp"


void Resolve::init()
{
	Parser::init();

	DeclarationNode& root = decls.emplace_back();
	root.parent = 0xFFFF'FFFF;
	root.ast_node = 0;
	root.name = "global";
}

bool Resolve::gatherUnorderedDeclarations(DeclNodeIndex parent_decl_idx, AST_NodeIndex ast_parent_idx)
{
	AST_BaseNode* ast_parent = getBaseNode(ast_parent_idx);

	for (AST_NodeIndex ast_child_idx : ast_parent->children) {

		AST_Node& ast_node = nodes[ast_child_idx];

		if (std::holds_alternative<AST_SourceFile>(ast_node)) {

			auto& ast_source_file = std::get<AST_SourceFile>(ast_node);

			addDeclaration(parent_decl_idx, ast_child_idx, "", DeclarationType::source_file,
				ast_source_file.decl_node);

			if (gatherUnorderedDeclarations(ast_source_file.decl_node, ast_child_idx) == false) {
				return false;
			}
		}
		else if (std::holds_alternative<AST_VariableDeclaration>(ast_node)) {

			auto& ast_var_decl = std::get<AST_VariableDeclaration>(ast_node);

			if (addDeclaration(parent_decl_idx, ast_child_idx, ast_var_decl.name.value, DeclarationType::variable,
				ast_var_decl.decl_node) == false)
			{
				return false;
			}
		}
		else if (std::holds_alternative<AST_TypeDeclaration>(ast_node)) {

			auto& ast_type_decl = std::get<AST_TypeDeclaration>(ast_node);

			if (addDeclaration(parent_decl_idx, ast_child_idx,
				ast_type_decl.name.value, DeclarationType::type, ast_type_decl.decl_node))
			{
				// @TODO: 
			}
			else {
				return false;
			}
		}
		else if (std::holds_alternative<AST_OperatorOverload>(ast_node)) {

			auto& ast_op_overload = std::get<AST_OperatorOverload>(ast_node);

			if (addDeclaration(0, ast_child_idx,
				ast_op_overload.op.value, DeclarationType::operator_overload,
				ast_op_overload.decl_node) == false)
			{
				return false;
			}
		}
		else if (std::holds_alternative<AST_FunctionImplementation>(ast_node)) {

			auto& ast_func_impl = std::get<AST_FunctionImplementation>(ast_node);

			if (addDeclaration(parent_decl_idx, ast_child_idx,
				ast_func_impl.name.back().value, DeclarationType::function,
				ast_func_impl.decl_node) == false)
			{
				return false;
			}
		}
	}

	return true;
}

DeclNodeIndex Resolve::findDeclaration(DeclarationNode& parent, uint32_t max_child_idx,
	std::string name)
{
	for (DeclNodeIndex i = 0; i < max_child_idx; i++) {

		DeclNodeIndex child_decl_idx = parent.children[i];
		DeclarationNode& child = decls[child_decl_idx];

		if (child.name == name) {
			return child_decl_idx;
		}
	}

	return 0xFFFF'FFFF;
}

DeclNodeIndex Resolve::resolveAdress(DeclarationStack& starting_stack, std::vector<Token>& adress, DeclarationType type)
{
	DeclNodeIndex r_decl_idx = 0xFFFF'FFFF;

	DeclarationStack* stack = &starting_stack;
	uint32_t last_adress_idx = adress.size() - 1;
	uint32_t adress_idx = last_adress_idx;

	while (stack != nullptr) {

		DeclarationNode& parent_decl = decls[stack->decl];

		DeclNodeIndex decl_idx = findDeclaration(parent_decl, stack->max_child_idx, adress[adress_idx].value);

		if (decl_idx != 0xFFFF'FFFF && decls[decl_idx].type == type) {

			// save for later
			if (adress_idx == last_adress_idx) {
				r_decl_idx = decl_idx;
			}

			if (adress_idx == 0) {
				return r_decl_idx;
			}
			else {
				adress_idx--;
			}
		}
		// break the chain, reset the lookup
		else {
			adress_idx = last_adress_idx;
		}

		stack = stack->prev;
	}

	return 0xFFFF'FFFF;
}

//bool Resolve::resolveType(DeclarationStack& parent_stack, AST_NodeIndex ast_node_idx)
//{
//	auto* type = getNode<AST_Type>(ast_node_idx);
//
//	return true;
//}

DeclNodeIndex Resolve::_resolveExpression(DeclarationStack& parent_stack, AST_NodeIndex ast_node_idx)
{
	auto& ast_node = nodes[ast_node_idx];

	if (std::holds_alternative<AST_Literal>(ast_node)) {

		auto* lit = getNode<AST_Literal>(ast_node_idx);

		switch (lit->token.type) {
		// Integer
		case TokenTypes::i32: {
			return i32_decl;
		}
		case TokenTypes::u32: {
			return u32_decl;
		}
		// Float
		case TokenTypes::f32: {
			return f32_decl;
		}
		// String
		case TokenTypes::STRING: {
			return string_decl;
		}
		default: throw;
		}
	}
	//else if (std::holds_alternative<AST_Variable>(ast_node)) {

	//	auto* var = getNode<AST_Variable>(ast_node_idx);

	//	// find variable declaration
	//	DeclNodeIndex var_decl_idx = resolveAdress(parent_stack, var->address, DeclarationType::variable);
	//	if (var_decl_idx != ast_invalid_idx) {

	//		DeclarationNode& var_decl = decls[var_decl_idx];
	//		var->ast_var_decl = var_decl.ast_node;

	//		auto* ast_var_decl = getNode<AST_VariableDeclaration>(var_decl.ast_node);
	//		auto* ast_type = getNode<AST_Type>(ast_var_decl->type)
	//		return 
	//	}

	//	throw;
	//}
	else if (std::holds_alternative<AST_BinaryOperator>(ast_node)) {

		auto* binary_op = getNode<AST_BinaryOperator>(ast_node_idx);

		DeclNodeIndex left_decl_idx = _resolveExpression(parent_stack, binary_op->children[0]);
		if (left_decl_idx == ast_invalid_idx) {
			return ast_invalid_idx;
		}
		
		DeclNodeIndex right_decl_idx = _resolveExpression(parent_stack, binary_op->children[1]);
		if (right_decl_idx == ast_invalid_idx) {
			return ast_invalid_idx;
		}

		// find operator overload
		{
			for (DeclNodeIndex decl_node_idx : decls[0].children) {

				DeclarationNode& decl_node = decls[decl_node_idx];

				/*if () {

				}*/
			}
		}

		if (left_decl_idx == right_decl_idx) {
			return left_decl_idx;
		}
		else {
			CompilerMessage& message = messages.emplace_back();
			message.severity = MessageSeverity::Error;

			MessageRow& err = message.rows.emplace_back();
			err.text = std::format(
				"Could not find a binary operator '{0}' that takes the following types:",
				binary_op->token.value
			);
			err.selection = binary_op->token;

			DeclarationNode& left_decl = decls[left_decl_idx];
			DeclarationNode& right_decl = decls[right_decl_idx];

			MessageRow& left_type = message.rows.emplace_back();
			left_type.text = getFullName(left_decl_idx);

			if (left_decl.ast_node != ast_invalid_idx) {
				left_type.selection = getBaseNode(left_decl.ast_node)->selection;
			}

			MessageRow& right_type = message.rows.emplace_back();
			right_type.text = getFullName(right_decl_idx);

			if (right_decl.ast_node != ast_invalid_idx) {
				right_type.selection = getBaseNode(right_decl.ast_node)->selection;
			}

			return ast_invalid_idx;
		}
	}
	else throw;
}

DeclNodeIndex Resolve::resolveExpression(DeclarationStack& parent_stack, AST_NodeIndex ast_node_idx)
{
	auto* expression = getNode<AST_Expression>(ast_node_idx);

	return _resolveExpression(parent_stack, expression->children[0]);
}

bool Resolve::resolveStatements(DeclarationStack& parent_stack, AST_NodeIndex ast_node_idx)
{
	auto* statements = getNode<AST_Statements>(ast_node_idx);

	DeclNodeIndex statements_decl_idx;
	addDeclaration(parent_stack.decl, ast_node_idx, "", DeclarationType::scope, statements_decl_idx);

	DeclarationStack stack;
	stack.prev = &parent_stack;
	stack.decl = statements_decl_idx;
	stack.max_child_idx = 0;

	for (AST_NodeIndex ast_statement_idx : statements->children) {

		AST_Node& statement = nodes[ast_statement_idx];

		if (std::holds_alternative<AST_VariableDeclaration>(statement)) {

			auto* ast_var_decl = getNode<AST_VariableDeclaration>(ast_statement_idx);
			
			DeclNodeIndex var_decl_idx;

			if (addDeclaration(statements_decl_idx, ast_statement_idx, ast_var_decl->name.value, DeclarationType::variable,
				var_decl_idx) == false)
			{
				return false;
			}

			auto* ast_type = getNode<AST_Type>(ast_var_decl->type);

			DeclNodeIndex type_decl_idx = resolveAdress(stack, ast_type->address, DeclarationType::type);
			if (type_decl_idx == 0xFFFF'FFFF) {

				logResolveError(std::format(
					"Unresolved type '{}' in variable declaration",
					getAdressName(ast_type->address)),
					ast_type->address
				);

				return false;
			}

			if (ast_var_decl->default_expr != ast_invalid_idx) {

				DeclNodeIndex expr_decl_idx = resolveExpression(stack, ast_var_decl->default_expr);
				if (expr_decl_idx != ast_invalid_idx) {


				}
				else {
					return ast_invalid_idx;
				}
			}

			stack.max_child_idx++;
		}
		else if (std::holds_alternative<AST_VariableAssignment>(statement)) {

			auto* assignment = getNode<AST_VariableAssignment>(ast_statement_idx);
			assignment->decl_node = resolveAdress(stack, assignment->address, DeclarationType::variable);

			if (assignment->decl_node == 0xFFFF'FFFF) {

				logResolveError(std::format(
					"Unresolved destination variable '{}' in assignment",
					getAdressName(assignment->address)),
					assignment->address
				);

				return false;
			}
		}
	}

	return true;
}

bool Resolve::resolveFunctionImplementation(DeclarationStack& parent_stack, AST_NodeIndex ast_node_idx)
{
	auto* ast_func_impl = getNode<AST_FunctionImplementation>(ast_node_idx);

	DeclarationStack stack;
	stack.prev = &parent_stack;
	stack.decl = ast_func_impl->decl_node;
	stack.max_child_idx = 0;

	return resolveStatements(stack, ast_func_impl->statements);;
}

//bool Resolve::resolveTypeDeclaration(DeclarationStack& parent_stack, AST_NodeIndex ast_node)
//{
//
//}

bool Resolve::resolveSourceFile(DeclarationStack& parent_stack, AST_NodeIndex ast_node_idx)
{
	AST_SourceFile* ast_source_file = getNode<AST_SourceFile>(ast_node_idx);

	for (AST_NodeIndex ast_child_idx : ast_source_file->children) {

		AST_Node& ast_node = nodes[ast_child_idx];

		if (std::holds_alternative<AST_FunctionImplementation>(ast_node)) {

			if (resolveFunctionImplementation(parent_stack, ast_child_idx) == false) {
				return false;
			}
		}
	}

	return true;
}

bool Resolve::resolve()
{
	{
		{
			AST_NodeIndex ast_node_idx;
			auto* ast_type_decl = addNode<AST_TypeDeclaration>(ast_invalid_idx, ast_node_idx);
			ast_type_decl->name.value = "i32";
			ast_type_decl->name.type = TokenTypes::IDENTIFIER;

			addDeclaration(0, ast_node_idx, ast_type_decl->name.value, DeclarationType::type,
				i32_decl);

			ast_type_decl->decl_node = i32_decl;
		}

		{
			AST_NodeIndex ast_node_idx;
			auto* ast_type_decl = addNode<AST_TypeDeclaration>(ast_invalid_idx, ast_node_idx);
			ast_type_decl->name.value = "u32";
			ast_type_decl->name.type = TokenTypes::IDENTIFIER;

			addDeclaration(0, ast_node_idx, ast_type_decl->name.value, DeclarationType::type,
				u32_decl);

			ast_type_decl->decl_node = u32_decl;
		}

		{
			AST_NodeIndex ast_node_idx;
			auto* ast_type_decl = addNode<AST_TypeDeclaration>(ast_invalid_idx, ast_node_idx);
			ast_type_decl->name.value = "f32";
			ast_type_decl->name.type = TokenTypes::IDENTIFIER;

			addDeclaration(0, ast_node_idx, ast_type_decl->name.value, DeclarationType::type,
				f32_decl);

			ast_type_decl->decl_node = f32_decl;
		}

		{
			AST_NodeIndex ast_node_idx;
			auto* ast_type_decl = addNode<AST_TypeDeclaration>(ast_invalid_idx, ast_node_idx);
			ast_type_decl->name.value = "string";
			ast_type_decl->name.type = TokenTypes::IDENTIFIER;

			addDeclaration(0, ast_node_idx, ast_type_decl->name.value, DeclarationType::type,
				string_decl);

			ast_type_decl->decl_node = string_decl;
		}
	}

	if (gatherUnorderedDeclarations(0, 0) == false) {
		return false;
	}

	DeclarationStack stack;
	stack.prev = nullptr;
	stack.decl = 0;
	stack.max_child_idx = decls[0].children.size();

	auto* ast_root = getNode<AST_Root>(0);

	for (AST_NodeIndex ast_source_file_idx : ast_root->children) {

		if (resolveSourceFile(stack, ast_source_file_idx) == false) {
			return false;
		}
	}

	return true;
}

void Resolve::_printDeclarations(DeclNodeIndex decl_idx, uint32_t depth)
{
	DeclarationNode* decl = &decls[decl_idx];
	
	for (uint32_t i = 0; i < depth; i++) {
		printf("  ");
	}

	printf("%s \n",
		decl->name.c_str()
	);

	for (auto child_decl_idx : decl->children) {
		_printDeclarations(child_decl_idx, depth + 1);
	}
}

void Resolve::printDeclarations()
{
	printf("Declaration Node Tree: \n");
	_printDeclarations(0, 0);
}
