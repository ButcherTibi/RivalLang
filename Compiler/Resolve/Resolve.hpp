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
	operator_overload,
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


/* This is not done.
* The Resolve stage is used to locate declaration and implementation of stuff.
* In a more complex way, a programing language that support operator overloading does not have type errors
* in a strict sense it simply has not found a coresponding operator overload that matches that usage. */
class Resolve : public Parser {
public:
	std::vector<DeclarationNode> decls;


	/* Basic Types */

	// Integer
	DeclNodeIndex i32_decl;
	//DeclNodeIndex i64_decl;
	DeclNodeIndex u32_decl;
	//DeclNodeIndex u64_decl;

	// Float
	DeclNodeIndex f32_decl;
	//DeclNodeIndex f64_decl;

	DeclNodeIndex string_decl;

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

	DeclNodeIndex _resolveExpression(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	DeclNodeIndex resolveExpression(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	// bool resolveType(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	bool resolveStatements(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	bool resolveFunctionImplementation(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	// bool resolveTypeDeclaration(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	bool resolveSourceFile(DeclarationStack& parent_stack, AST_NodeIndex ast_node);


	/* Main */

	bool resolve();

	
	/* Resolve Error */

	void logResolveError(std::string text, std::vector<Token>& adress);


	// Debug
	void _printDeclarations(DeclNodeIndex scope_idx, uint32_t depth);
	void printDeclarations();
};
