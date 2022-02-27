
// Header
#include "Parser.hpp"


DeclarationNode* Parser::addDeclaration(DeclNodeIndex parent_scope_idx, DeclNodeIndex& r_new_scope_idx)
{
	r_new_scope_idx = decls.size();

	DeclarationNode* parent = &decls[parent_scope_idx];
	parent->children.push_back(r_new_scope_idx);

	DeclarationNode* child = &decls.emplace_back();
	child->parent = parent_scope_idx;

	return child;
}

DeclarationNode* Parser::addDeclaration(DeclNodeIndex parent_scope_idx)
{
	DeclNodeIndex lazyness;
	return addDeclaration(parent_scope_idx, lazyness);
}

void Parser::gatherUnorderedDeclarations(DeclNodeIndex parent_scope, AST_NodeIndex ast_parent_idx)
{
	AST_BaseNode* ast_parent = getBaseNode(ast_parent_idx);

	for (AST_NodeIndex ast_child_idx : ast_parent->children) {

		AST_Node& ast_node = nodes[ast_child_idx];

		if (std::holds_alternative<AST_SourceFile>(ast_node)) {

			DeclNodeIndex source_file_idx;
			DeclarationNode* new_decl = addDeclaration(parent_scope, source_file_idx);
			new_decl->ast_node = ast_child_idx;
			new_decl->scope_type = ScopeType::DECLARATIONS;

			auto& ast_source_file = std::get<AST_SourceFile>(ast_node);
			ast_source_file.decl_node = source_file_idx;

			gatherUnorderedDeclarations(source_file_idx, ast_child_idx);
		}
		else if (std::holds_alternative<AST_VariableDeclaration>(ast_node)) {

			auto& ast_var_decl = std::get<AST_VariableDeclaration>(ast_node);

			DeclarationNode* new_decl = addDeclaration(parent_scope, ast_var_decl.decl_node);
			new_decl->ast_node = ast_child_idx;
			new_decl->name = ast_var_decl.name.value;
			new_decl->scope_type = ScopeType::NO_SCOPE;
		}
		else if (std::holds_alternative<AST_FunctionImplementation>(ast_node)) {

			auto& ast_func_impl = std::get<AST_FunctionImplementation>(ast_node);

			DeclNodeIndex func_impl_idx;
			DeclarationNode* new_decl = addDeclaration(parent_scope, func_impl_idx);
			new_decl->ast_node = ast_child_idx;
			new_decl->name = ast_func_impl.name.back().value;
			new_decl->scope_type = ScopeType::STATEMENTS;

			ast_func_impl.decl_node = func_impl_idx;
		}
	}
}

DeclarationNode* Parser::getParentDeclaration()
{
	DeclNodeIndex decl_node_idx = stacks.back().decl_node_idx;
	return &decls[decl_node_idx];
}

DeclarationStack* Parser::addDeclarationStack(DeclNodeIndex decl_node_idx)
{
	DeclarationStack& new_stack = stacks.emplace_back();
	new_stack.decl_node_idx = decl_node_idx;
	new_stack.stack_pos = 0;

	return &new_stack;
}

DeclNodeIndex Parser::resolveAdress(std::vector<Token>& adress)
{
	uint32_t adress_idx = adress.size() - 1;

	for (int32_t stacks_idx = stacks.size() - 1; stacks_idx >= 0; stacks_idx--) {

		DeclarationStack& stack = stacks[stacks_idx];
		DeclarationNode& parent_scope = decls[stack.decl_node_idx];

		for (int32_t i = stack.stack_pos - 1; i >= 0; i--) {

			DeclNodeIndex decl_node_idx = parent_scope.children[i];
			DeclarationNode& declaration = decls[decl_node_idx];

			if (declaration.name != "") {

				if (declaration.name == adress[adress_idx].value) {

					if (adress_idx == 0) {
						return decl_node_idx;
					}
					else {
						adress_idx--;
					}
				}
				else {
					adress_idx = adress.size() - 1;
				}
			}
		}
	}

	return 0xFFFF'FFFF;
}

bool Parser::resolveStatements(AST_NodeIndex ast_node_idx)
{
	auto* statements = getNode<AST_Statements>(ast_node_idx);

	DeclarationStack* new_stack;
	{	
		DeclNodeIndex new_decl_idx;
		DeclarationNode* new_decl = addDeclaration(stacks.back().decl_node_idx, new_decl_idx);
		new_decl->ast_node = ast_node_idx;
		new_decl->scope_type = ScopeType::STATEMENTS;

		new_stack = addDeclarationStack(new_decl_idx);
	}

	for (AST_NodeIndex ast_statement_idx : statements->children) {

		AST_Node& statement = nodes[ast_statement_idx];

		if (std::holds_alternative<AST_VariableDeclaration>(statement)) {

			auto* var_decl = getNode<AST_VariableDeclaration>(ast_statement_idx);
			
			DeclarationNode* new_decl = addDeclaration(new_stack->decl_node_idx);
			new_decl->ast_node = ast_statement_idx;
			new_decl->name = var_decl->name.value;
			new_decl->scope_type = ScopeType::NO_SCOPE;

			new_stack->stack_pos++;
		}
		else if (std::holds_alternative<AST_VariableAssignment>(statement)) {

			auto* assignment = getNode<AST_VariableAssignment>(ast_statement_idx);
			assignment->decl_node = resolveAdress(assignment->address);

			if (assignment->decl_node == 0xFFFF'FFFF) {
				__debugbreak();
				return false;
			}
		}
		else {
			throw;
		}
	}

	stacks.pop_back();

	return true;
}

bool Parser::resolveFunctionImplementation(AST_NodeIndex ast_node_idx)
{
	auto* ast_func_impl = getNode<AST_FunctionImplementation>(ast_node_idx);

	addDeclarationStack(ast_func_impl->decl_node);

	bool result = resolveStatements(ast_func_impl->statements);

	stacks.pop_back();

	return result;
}

bool Parser::resolve()
{
	auto* ast_root = getNode<AST_Root>(0);

	gatherUnorderedDeclarations(0, 0);

	std::vector<DeclarationStack> decl_stack;
	{
		auto& root = decl_stack.emplace_back();
		root.decl_node_idx = 0;
		root.stack_pos = ast_root->children.size();
	}

	for (AST_NodeIndex source_file_idx : ast_root->children) {

		auto* ast_src_file = getNode<AST_SourceFile>(source_file_idx);

		if (resolveFunctionImplementation(ast_src_file->children[0]) == false) {
			return false;
		}
	}

	return true;
}

void Parser::_printScopes(DeclNodeIndex scope_idx, uint32_t depth)
{
	DeclarationNode* scope = &decls[scope_idx];
	
	for (uint32_t i = 0; i < depth; i++) {
		printf("  ");
	}

	printf("%s \n",
		scope->name.c_str()
	);

	for (auto child_scope_id : scope->children) {
		_printScopes(child_scope_id, depth + 1);
	}
}

void Parser::printDeclarations()
{
	printf("DeclarationNode Tree: \n");
	_printScopes(0, 0);
}
