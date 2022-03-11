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

struct DeclarationNode {
	DeclNodeIndex parent;
	std::map<std::string, DeclNodeIndex> children;

	AST_NodeIndex ast_node;
	std::string name;
};

struct DeclarationStack {
	DeclarationStack* prev;

	DeclNodeIndex decl;
	uint32_t max_child_idx;
};


class Resolve : public Parser {
public:
	std::vector<DeclarationNode> decls;

	// used internally during second pass
	// std::vector<DeclarationStack> stacks;

public:
	void init();


	/* Utils */

	DeclarationNode* getDeclaration(DeclNodeIndex decl_idx);
	std::string getFullName(DeclNodeIndex decl_idx);

	template<typename T = AST_Root>
	bool addDeclaration(DeclNodeIndex parent_decl_idx,
		AST_NodeIndex ast_node_idx, std::string name,
		DeclNodeIndex& r_new_decl)
	{
		r_new_decl = decls.size();

		DeclarationNode* parent = &decls[parent_decl_idx];

		auto add_declaration = [&]() {
			parent->children.insert({ name, r_new_decl });

			DeclarationNode* child = &decls.emplace_back();
			child->parent = parent_decl_idx;
			child->ast_node = ast_node_idx;
			child->name = name;

			/*if (std::is_same<>) {

			}*/
		};

		if (name == "") {
			name = "_ "s + std::to_string(parent->children.size());

			add_declaration();
			return true;
		}
		else if (parent->children.contains(name) == false) {
			add_declaration();
			return true;
		}
		else {
			CompilerMessage& message = messages.emplace_back();
			message.severity = MessageSeverity::Error;

			MessageRow& rule = message.rows.emplace_back();
			rule.text = std::format(
				"Cannot create a declaration with the same name '{}' as a previous declaration.",
				name);
			rule.selection = getNodeSelection(ast_node_idx);

			{
				auto* prev_decl = getDeclaration(parent->children.find(name)->second);

				MessageRow& prev = message.rows.emplace_back();
				prev.text = std::format("Previous declaration link.");
				prev.selection = getNodeSelection(prev_decl->ast_node);
			}

			return false;
		}
	}


	/* First Pass */

	bool gatherUnorderedDeclarations(DeclNodeIndex parent_scope, AST_NodeIndex ast_parent_idx);


	/* Second Pass */

	DeclNodeIndex resolveAdress(DeclarationStack& stack, std::vector<Token>& adress);

	// bool resolveExpression(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	bool resolveStatements(DeclarationStack& parent_stack, AST_NodeIndex ast_node);

	bool resolveFunctionImplementation(DeclarationStack& parent_stack, AST_NodeIndex ast_node);


	/* Main */

	bool resolve();


	// Debug
	void _printDeclarations(DeclNodeIndex scope_idx, uint32_t depth);
	void printDeclarations();
};
