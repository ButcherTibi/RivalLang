
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

			addDeclaration<AST_SourceFile>(parent_decl_idx, ast_child_idx,
				"", ast_source_file.decl_node);

			if (gatherUnorderedDeclarations(ast_source_file.decl_node, ast_child_idx) == false) {
				return false;
			}
		}
		else if (std::holds_alternative<AST_VariableDeclaration>(ast_node)) {

			auto& ast_var_decl = std::get<AST_VariableDeclaration>(ast_node);

			if (addDeclaration<AST_VariableDeclaration>(parent_decl_idx, ast_child_idx,
				ast_var_decl.name.value, ast_var_decl.decl_node) == false)
			{
				return false;
			}
		}
		else if (std::holds_alternative<AST_FunctionImplementation>(ast_node)) {

			auto& ast_func_impl = std::get<AST_FunctionImplementation>(ast_node);

			if (addDeclaration<AST_FunctionImplementation>(parent_decl_idx, ast_child_idx,
				ast_func_impl.name.back().value, ast_func_impl.decl_node) == false)
			{
				return false;
			}
		}
	}

	return true;
}

DeclNodeIndex Resolve::resolveAdress(DeclarationStack& starting_stack, std::vector<Token>& adress)
{
	DeclarationStack* stack = &starting_stack;
	uint32_t adress_idx = adress.size() - 1;

	while (stack != nullptr) {

		DeclarationNode& parent_decl = decls[stack->decl];

		bool found_name = false;
		uint32_t i = 0;

		for (auto& [name, decl_idx] : parent_decl.children) {

			if (name != "" && name == adress[adress_idx].value) {

				if (adress_idx == 0) {
					return decl_idx;
				}
				else {
					adress_idx--;
					found_name = true;
					break;
				}
			}

			i++;

			if (i >= stack->max_child_idx) {
				break;
			}
		}

		if (found_name == false) {
			adress_idx = adress.size() - 1;
		}

		stack = stack->prev;
	}

	return 0xFFFF'FFFF;
}

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
	addDeclaration<AST_Statements>(parent_stack.decl, ast_node_idx, "", statements_decl_idx);

	DeclarationStack stack;
	stack.prev = &parent_stack;
	stack.decl = statements_decl_idx;
	stack.max_child_idx = 0;

	for (AST_NodeIndex ast_statement_idx : statements->children) {

		AST_Node& statement = nodes[ast_statement_idx];

		if (std::holds_alternative<AST_VariableDeclaration>(statement)) {

			auto* ast_var_decl = getNode<AST_VariableDeclaration>(ast_statement_idx);
			
			DeclNodeIndex var_decl_idx;

			if (addDeclaration<AST_VariableDeclaration>(statements_decl_idx, ast_statement_idx,
				ast_var_decl->name.value, var_decl_idx) == false)
			{
				return false;
			}

			stack.max_child_idx++;
		}
		else if (std::holds_alternative<AST_VariableAssignment>(statement)) {

			auto* assignment = getNode<AST_VariableAssignment>(ast_statement_idx);
			assignment->decl_node = resolveAdress(stack, assignment->address);

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
		else {
			__debugbreak();
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
		// addDeclaration(0, 0xFFFF'FFFF, "u64", stub);
		// addDeclaration(0, 0xFFFF'FFFF, "string", stub);
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

	for (auto child_decl : decl->children) {
		_printDeclarations(child_decl.second, depth + 1);
	}
}

void Resolve::printDeclarations()
{
	printf("Declaration Node Tree: \n");
	_printDeclarations(0, 0);
}
