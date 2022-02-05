#pragma once

// Mine
#include "Parser.hpp"


//struct TypeNode {
//	// the name of a type includes namespaces or parent classes
//	// namespace.class_name != class_name != namespace.namespace.class_name
//	std::vector<std::string> name;
//};

enum class NamingLevel {
	NAMESPACE,
	CLASS,
	FUNCTION
};

struct TypeName {
	// namespace, class or primitve
	std::string name;
	NamingLevel level;

	std::vector<uint32_t> children;
};


struct ContextVariable {
	std::string var_name;
	std::vector<uint32_t> var_type;
	
};

struct AccessibilityContext {
	// what variables are available to be referenced
	std::vector<ContextVariable> variables;

	// nodes to be processed
	std::vector<AST_Node*> nodes;
};


class TypeResolution {
public:
	std::vector<Token> tokens;
	std::vector<AST_Node> nodes;

	std::vector<TypeName> types;

public:

	void init()
	{
		auto& u64_type = types.emplace_back();
		u64_type.name = { "u64" };

		auto& f32_type = types.emplace_back();
		f32_type.name = { "f32" };
	}

	/*uint32_t findTypeIndex(std::vector<std::string>& name)
	{
		std::vector<uint32_t> now = { 0 };
		std::vector<uint32_t> next;

		while () {

		}
	}*/

	Token& getToken(uint32_t token_index)
	{
		return tokens[token_index];
	}

	template<typename T>
	T* getNode(uint32_t node_idx)
	{
		return std::get_if<AST_Type>(&nodes[node_idx]);
	}

	void f(std::vector<>)
	{

	}
	
	void f()
	{
		std::vector<AccessibilityContext> now_contexts;
		{
			auto& root = now_contexts.emplace_back();
			root.nodes.push_back(&nodes.front());
		}

		std::vector<AccessibilityContext> next_contexts;

		while (now_contexts.size()) {

			for (AccessibilityContext& now_context: now_contexts) {

				for (AST_Node* node : now_context.nodes) {

					if (std::holds_alternative<AST_SourceFile>(*node)) {

						
					}
					else if (std::holds_alternative<AST_VariableDeclaration>(*node)) {

						auto& var_decl = std::get<AST_VariableDeclaration>(*node);
						Token& name = tokens[var_decl.name_token];

						auto& type_node = std::get<AST_Type>(nodes[var_decl.children[0]]);
						Token& type = getToken(type_node.name_token);

						ContextVariable& new_ctx_var = now_context.variables.emplace_back();
						new_ctx_var.var_name = name.value;
						new_ctx_var.var_type = { 0 };
					}
					else if (std::holds_alternative<AST_VariableAssignment>(*node)) {

						auto& assignment = std::get<AST_VariableAssignment>(*node);

						Token& name = getToken(assignment.name_tokens[0]);
						
					}
				}
			}
		}
	}
};
