
// Header
#include "Resolve.hpp"


DeclarationNode* Resolve::getDeclaration(DeclNodeIndex decl_idx)
{
	return &decls[decl_idx];
}

std::string Resolve::getFullName(DeclNodeIndex decl_idx)
{
	std::string full_name;

	auto isUnNamed = [](std::string& name) {

		if (name.size() > 2 && name[0] == '_' && name[1] == ' ') {
			return true;
		}

		return false;
	};

	while (decl_idx != 0xFFFF'FFFF) {

		DeclarationNode* node = getDeclaration(decl_idx);

		if (isUnNamed(node->name) == false) {

			if (full_name.empty() == false) {
				full_name.insert(0, node->name + "."s);
			}
			else {
				full_name.insert(0, node->name);
			}
		}

		decl_idx = node->parent;
	}

	return full_name;
}
//
//bool Resolve::addDeclaration(DeclNodeIndex parent_decl_idx,
//	AST_NodeIndex ast_node_idx, std::string name,
//	DeclNodeIndex& r_new_decl)
//{
//	r_new_decl = decls.size();
//
//	DeclarationNode* parent = &decls[parent_decl_idx];
//
//	auto add_declaration = [&]() {
//		parent->children.insert({ name, r_new_decl });
//
//		DeclarationNode* child = &decls.emplace_back();
//		child->parent = parent_decl_idx;
//		child->ast_node = ast_node_idx;
//		child->name = name;
//	};
//
//	if (name == "") {
//		name = "_ "s + std::to_string(parent->children.size());
//
//		add_declaration();
//		return true;
//	}
//	else if (parent->children.contains(name) == false) {
//		add_declaration();
//		return true;
//	}
//	else {
//		CompilerMessage& message = messages.emplace_back();
//		message.severity = MessageSeverity::Error;
//
//		MessageRow& rule = message.rows.emplace_back();
//		rule.text = std::format(
//			"Cannot create a declaration with the same name '{}' as a previous declaration.",
//			name);
//		rule.selection = getNodeSelection(ast_node_idx);
//
//		{
//			auto* prev_decl = getDeclaration(parent->children.find(name)->second);
//
//			MessageRow& prev = message.rows.emplace_back();
//			prev.text = std::format("Previous declaration link.");
//			prev.selection = getNodeSelection(prev_decl->ast_node);
//		}
//
//		return false;
//	}
//}
