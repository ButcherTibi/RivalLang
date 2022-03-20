
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

	while (decl_idx != 0) {

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

bool Resolve::addDeclaration(DeclNodeIndex parent_decl_idx,
	AST_NodeIndex ast_node_idx, std::string name, DeclarationType type,
	DeclNodeIndex& r_new_decl)
{
	r_new_decl = decls.size();

	DeclarationNode* parent = &decls[parent_decl_idx];

	auto add_declaration = [&]() {
		parent->children.push_back(r_new_decl);

		DeclarationNode* child = &decls.emplace_back();
		child->parent = parent_decl_idx;
		child->ast_node = ast_node_idx;
		child->name = name;
		child->type = type;
	};

	if (type == DeclarationType::scope || type == DeclarationType::source_file) {

		name = "_ "s + std::to_string(parent->children.size());

		add_declaration();
		return true;
	}
	else {

		for (auto& child_decl_idx : parent->children) {

			DeclarationNode* child = getDeclaration(child_decl_idx);

			// duplicate declaration found
			if (child->name == name) {

				CompilerMessage& message = messages.emplace_back();
				message.severity = MessageSeverity::Error;

				MessageRow& rule = message.rows.emplace_back();
				rule.text = std::format(
					"Cannot create a declaration with the same name '{}' as a previous declaration.",
					name);
				rule.selection = getNodeSelection(ast_node_idx);

				{
					auto* prev_decl = getDeclaration(child_decl_idx);

					MessageRow& prev = message.rows.emplace_back();
					prev.text = std::format("Previous declaration link.");
					prev.selection = getNodeSelection(prev_decl->ast_node);
				}

				return false;
			}
		}

		// no duplicate declaration found
		add_declaration();
		return true;
	}
}

void Resolve::logResolveError(std::string text, std::vector<Token>& adress)
{
	CompilerMessage& message = messages.emplace_back();
	message.severity = MessageSeverity::Error;

	MessageRow& row = message.rows.emplace_back();
	row.text = text;
	row.selection = adress;
}
