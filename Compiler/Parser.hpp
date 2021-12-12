#pragma once

// Standard
#include <variant>

// Mine
#include "Lexer.hpp"


enum class AST_NodeTypes {
	ROOT,

	EXPRESSION,
	TYPE,
	VARIABLE_DECLARATION,
	SCOPE,

	// FUNCTION_SIGNATURE,
	FUNCTION_DEFINITION,
	FUNCTION_CALL
};

struct AST_NodeBase {
	uint32_t parent;
	std::vector<uint32_t> children;

	AST_NodeTypes ast_node_type;

	virtual std::string toString(std::vector<Token>& tokens);
};


struct AST_FileRoot : AST_NodeBase {

};


struct AST_Type : AST_NodeBase {
	uint32_t name_token;

	std::string toString(std::vector<Token>& tokens) override;
};


struct AST_VariableDeclaration : AST_NodeBase {
	uint32_t name_token;  // name of the variable
	uint32_t symbol_token;  // usually a * to denote a pointer
	uint32_t type_node;  // root node of the type
	std::vector<uint32_t> keyword_tokens; // keywords to be applied


	std::string toString(std::vector<Token>& tokens) override;
};


struct AST_Scope : AST_NodeBase {

};


struct AST_FunctionDefinition : AST_NodeBase {
	uint32_t name_token;
	std::vector<uint32_t> param_nodes;
	uint32_t return_node;
	std::vector<uint32_t> keyword_tokens;
};


struct AST_FunctionCall : AST_NodeBase {
	std::vector<uint32_t> name;
};


typedef std::variant<
	AST_FileRoot,

	AST_Type,
	AST_VariableDeclaration,
	AST_Scope,
	AST_FunctionCall,

	AST_FunctionDefinition
> AST_Node;


struct CompilerErrorException {
	std::string msg;
	uint32_t line, column;

	CompilerErrorException(std::string new_msg, uint32_t new_line, uint32_t new_column)
	{
		msg = new_msg;
		line = new_line;
		column = new_column;
	}
};


class Parser {
public:
	std::vector<Token>* tokens;

	std::vector<AST_Node> nodes;

public:

	template<typename T>
	T* addNode(uint32_t& r_new_node_index)
	{
		r_new_node_index = nodes.size();

		return &nodes.emplace_back().emplace<T>();
	}

	template<typename T>
	T* getNode(uint32_t node_idx)
	{
		return std::get_if<AST_Type>(&nodes[node_idx]);
	}

	AST_NodeBase* getBaseNode(uint32_t node_idx);

	void linkParentAndChild(uint32_t parent_node_index, uint32_t child_node_index);


	/* Token traversal */

	// skip spacing and identifiers but not specified symbols to find symbol token
	uint32_t seekToSymbolToken(uint32_t start_token, std::string symbol_token,
		std::vector<std::string> not_allowed_symbols, bool allow_identifier = true);

	// skips only spacing to find symbol
	bool skipToSymbolToken(uint32_t& token_index, std::string target_symbol);

	// skips only spacing to find identifier
	bool skipToIdentifierToken(uint32_t& token_index);
	bool skipToIdentifierToken(uint32_t& token_index, std::string target_identifier);


	/* Parse Functions
	  @parent_node_index = parent to atach to
	  @start_token_index = poinat at which to start the look up
	  @end_token_index = point where the look up ended
	    will always be the next token after the statement/declaration/expression
	*/

	// parse the type in stuff like variable declarations, function parameters
	void parseType(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	void parseVariableDeclaration(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	void parseScope(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	void parseFunctionDefinition(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	void begin(FileToLex& file_to_lex);


	/* Check functions */

	bool isFunctionDefinition(uint32_t parent_token_index);


	/* Debug */
	void _print(uint32_t node_idx, uint32_t depth);
	void print();
};
