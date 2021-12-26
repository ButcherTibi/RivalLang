#pragma once

// Standard
#include <variant>

// Mine
#include "Lexer.hpp"


enum class AST_NodeTypes {
	FILE,

	OPERATOR_ADD_UNARY,
	OPERATOR_SUB_UNARY,
	OPERATOR_ADD_BINARY,
	OPERATOR_SUB_BINARY,
	OPERATOR_MUL,
	OPERATOR_DIV,
	OPERATOR_MOD,
	LITERAL,
	VARIABLE,

	EXPRESSION,
	STATEMENTS,

	TYPE,
	VARIABLE_DECLARATION,

	FUNCTION_DEFINITION,
	FUNCTION_CALL
};

struct AST_BaseNode {
	uint32_t parent = 0xFFFF'FFFF;
	std::vector<uint32_t> children;

	AST_NodeTypes ast_node_type;

	virtual std::string toString(std::vector<Token>& tokens);
};


struct AST_ExpressionSign : AST_BaseNode {
	// the sign is determined by node type
	//int32_t precedence;
};


struct AST_Literal : AST_BaseNode {
	uint32_t token;

	std::string toString(std::vector<Token>& tokens) override;
};


struct AST_Variable : AST_BaseNode {
	std::vector<uint32_t> name_tokens;  // name of the variable

	std::string toString(std::vector<Token>& tokens) override;
};


struct AST_FunctionCall : AST_BaseNode {
	std::vector<uint32_t> name_tokens;
	// expression node
};


struct AST_VariableDeclaration : AST_BaseNode {
	std::vector<uint32_t> name_tokens;  // name of the variable
	uint32_t symbol_token = 0xFFFF'FFFF;  // usually a * to denote a pointer

	uint32_t type_token;
	//std::vector<uint32_t> type_tree_tokens;  // type tree
	std::vector<uint32_t> keyword_tokens;  // keywords to be applied
	uint32_t expression_node = 0xFFFF'FFFF;

public:
	std::string toString(std::vector<Token>& tokens) override;
};


struct AST_FunctionDefinition : AST_BaseNode {
	uint32_t name_token;
	std::vector<uint32_t> param_nodes;
	uint32_t return_node;
	std::vector<uint32_t> keyword_tokens;
};


struct AST_Type : AST_BaseNode {
	uint32_t name_token;

	std::string toString(std::vector<Token>& tokens) override;
};


typedef std::variant<
	AST_BaseNode,
	AST_ExpressionSign,
	AST_Literal,
	AST_Variable,
	AST_FunctionCall,
	AST_VariableDeclaration,

	AST_FunctionDefinition,
	AST_Type
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

//
//struct SyntaxUnit {
//	uint32_t token;
//
//	std::vector<uint32_t> token_tree;
//
//	SyntaxUnit() = default;
//
//	SyntaxUnit(uint32_t new_token)
//	{
//		token = new_token;
//	}
//};

struct CompilerError {
	std::string msg;

	uint32_t line;
	uint32_t column;
};


class Parser {
public:
	std::vector<Token>* tokens;

	std::vector<AST_Node> nodes;

	std::vector<CompilerError> errors;

public:

	/* Utility functions */

	Token& getToken(uint32_t token_index)
	{
		return (*tokens)[token_index];
	}

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

	AST_BaseNode* getBaseNode(uint32_t node_idx);

	void linkParentAndChild(uint32_t parent_node_index, uint32_t child_node_index);


	/* Check Token */


	/* Seek Functions */

	// ski spacing and identifiers but not specified symbols to find symbol token
	bool seekToSymbolToken(uint32_t& token_index, std::string symbol_token,
		std::vector<std::string> not_allowed_symbols, bool allow_identifier = true);

	// skip spacing and identifiers to reach for symbol
	bool seekToSymbolToken(uint32_t& token_index, std::string symbol_token);


	/* Skip functions */

	// skip only spacing to find symbol
	bool skipToSymbolToken(uint32_t& token_index, std::string target_symbol);
	bool skipToSymbolToken(uint32_t token_index, std::string target_symbol, uint32_t& r_token_index);

	bool skipToExpressionSymbolToken(uint32_t token_index, uint32_t& r_token_index);
	bool skipToNumberToken(uint32_t token_index, uint32_t& r_token_index);
	bool skipToStringToken(uint32_t token_index, uint32_t& r_token_index);

	// skip anything to reach closing end symbol
	bool skipToClosingSymbolToken(uint32_t& token_index,
		std::string start_symbol_token, std::string end_symbol_token);

	// skip spacing and identifiers
	bool skipPastIdentifiers(uint32_t& token_index);

	// skip only spacing to find identifier
	bool skipToIdentifierToken(uint32_t& token_index);
	bool skipToIdentifierToken(uint32_t token_index, uint32_t& r_token_index);
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

	void parseExpr2(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	void parseName(uint32_t token_start,
		std::vector<uint32_t>& r_name, uint32_t& r_token_end);

	enum class SubExpressionStatus {

	};

	// parses stuff like ()
	bool parseSubExpression(uint32_t& token_index, int32_t parent_precedence,
		uint32_t& r_child_node_index);

	/*void parseExpr2(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);*/

	bool parseExpression(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	// parse stuff inside function body, stuff that can be executed
	bool parseStatements(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	void parseFunctionImplementation(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	void begin(FileToLex& file_to_lex);


	/* Check functions */

	// does not parse modifiers and is very permisive
	// Example: ComplexType<TemplateParam_1, TemplateParam_2><TemplateParam_1, TemplateParam_2>
	bool isType(uint32_t& token_index);

	bool isVariableDeclaration(uint32_t token_index);

	bool isFunctionCall(uint32_t token_index);

	bool isFunctionImplementation(uint32_t token_index);


	/* Error */

	void pushError(std::string error_mesage, uint32_t token_index);

	// "unexpected token {token.value} "
	void errorUnexpectedToken(std::string error_mesage, uint32_t token_index);


	/* Debug */
	void _print(uint32_t node_idx, uint32_t depth);
	void printTree();
	void printNodes(uint32_t start_index = 0, uint32_t end_index = 0xFFFF'FFFF);
};
