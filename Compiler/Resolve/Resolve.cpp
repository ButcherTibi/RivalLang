
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
		else if (std::holds_alternative<AST_FunctionImplementation>(ast_node)) {

			auto& ast_func_impl = std::get<AST_FunctionImplementation>(ast_node);

			if (addDeclaration(parent_decl_idx, ast_child_idx, ast_func_impl.name.back().value, DeclarationType::function,
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

//bool Resolve::resolveExpression(DeclarationStack& parent_stack, AST_NodeIndex ast_node_idx)
//{
//	auto* expression = getNode<AST_Expression>(ast_node_idx);
//
//	return true;
//}

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



			stack.max_child_idx++;
		}
		else if (std::holds_alternative<AST_VariableAssignment>(statement)) {

			auto* assignment = getNode<AST_VariableAssignment>(ast_statement_idx);
			assignment->decl_node = resolveAdress(stack, assignment->address, DeclarationType::variable);

			if (assignment->decl_node == 0xFFFF'FFFF) {

				CompilerMessage& message =  messages.emplace_back();
				message.severity = MessageSeverity::Error;

				MessageRow& row = message.rows.emplace_back();
				row.text = std::format(
					"Unresolved symbol '{}' in '{}'",
					getAdressName(assignment->address),
					getFullName(statements_decl_idx)
				);

				row.selection.start = assignment->address.front().selection.start;
				row.selection.end = assignment->address.back().selection.end;

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

bool Resolve::resolve()
{
	{
		DeclNodeIndex stub;
		addDeclaration(0, 0xFFFF'FFFF, "u64", DeclarationType::type, stub);
		addDeclaration(0, 0xFFFF'FFFF, "string", DeclarationType::type, stub);
	}

	gatherUnorderedDeclarations(0, 0);

	DeclarationStack stack;
	stack.prev = nullptr;
	stack.decl = 0;
	stack.max_child_idx = decls[0].children.size();

	auto* ast_root = getNode<AST_Root>(0);

	for (AST_NodeIndex source_file_idx : ast_root->children) {

		auto* ast_src_file = getNode<AST_SourceFile>(source_file_idx);

		if (resolveFunctionImplementation(stack, ast_src_file->children[0]) == false) {
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
