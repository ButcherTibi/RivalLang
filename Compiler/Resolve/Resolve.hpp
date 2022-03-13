#pragma once

// Standard
#include <set>

// Mine
#include "../Parser/Parser.hpp"


enum class ScopeType {
	DECLARATIONS,  // order independent
	STATEMENTS,  // order dependent
	NO_SCOPE  // does not restrict lookup
};

enum class DeclarationType {
	source_file,
	type,
	variable,
	function,
	scope,
};

struct DeclarationNode {
	DeclNodeIndex parent;
	std::vector<DeclNodeIndex> children;

	AST_NodeIndex ast_node;
	std::string name;
	DeclarationType type;
};

struct DeclarationStack {
	DeclarationStack* prev;

	DeclNodeIndex decl;
	uint32_t max_child_idx;
};


class Resolve : public Parser {
public:
	std::vector<DeclarationNode> decls;

public:
	void init();


	/* Utils */

	DeclarationNode* getDeclaration(DeclNodeIndex decl_idx);
	std::string getFullName(DeclNodeIndex decl_idx);

	bool addDeclaration(DeclNodeIndex parent_decl_idx,
		AST_NodeIndex ast_node_idx, std::string name, DeclarationType type,
		DeclNodeIndex& r_new_decl);


	/* First Pass */

	bool gatherUnorderedDeclarations(DeclNodeIndex parent_scope, AST_NodeIndex ast_parent_idx);


	/* Second Pass */

	DeclNodeIndex findDeclaration(DeclarationNode& parent, uint32_t max_child_idx,
		std::string name);

	DeclNodeIndex resolveAdress(DeclarationStack& stack, std::vector<Token>& adress, DeclarationType type);

	// bool resolveExpression(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	// bool resolveType(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	bool resolveStatements(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	bool resolveFunctionImplementation(DeclarationStack& parent_stack, AST_NodeIndex ast_node);


	/* Main */

	bool resolve();


	// Debug
	void _printDeclarations(DeclNodeIndex scope_idx, uint32_t depth);
	void printDeclarations();
};
