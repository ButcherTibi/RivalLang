#pragma once

// Standard
#include <variant>
#include <typeinfo>

// Mine
#include "Lexer.hpp"


// Base class for all abstract syntax tree nodes, never used on its own
struct AST_BaseNode {
	uint32_t parent = 0xFFFF'FFFF;
	std::vector<uint32_t> children;

	virtual std::string toString(std::vector<Token>&) {
		return "Node";
	};
};


// Code Spliting ////////////////////////////////////////////////////////////////////

struct AST_SourceFile : AST_BaseNode {
	std::string toString(std::vector<Token>&) override;
};


// Expression ///////////////////////////////////////////////////////////////////////

// Expression used in assignment or function call
struct AST_Expression : AST_BaseNode {
	std::string toString(std::vector<Token>&) override {
		return "Expression";
	};
};

// Binary Operators
struct AST_OperatorPlusBinary : AST_BaseNode {
	std::string toString(std::vector<Token>&) override {
		return "+";
	};
};

struct AST_OperatorMultiplication : AST_BaseNode {
	std::string toString(std::vector<Token>&) override {
		return "*";
	};
};

// Literals
struct AST_Literal : AST_BaseNode {
	uint32_t token;

	std::string toString(std::vector<Token>& tokens) override;
};

struct AST_NumericLiteral : AST_Literal { };
struct AST_StringLiteral : AST_Literal { };


// Variable /////////////////////////////////////////////////////////////////////////

struct AST_VariableDeclaration : AST_BaseNode {
	uint32_t name_token;  // name of the variable
	std::vector<uint32_t> modifiers_tokens;  // modifiers to be applied

public:
	std::string toString(std::vector<Token>& tokens) override;
};

struct AST_Variable : AST_BaseNode {
	std::vector<uint32_t> name_tokens;  // name of the variable

	std::string toString(std::vector<Token>& tokens) override;
};

struct AST_VariableAssignment : AST_BaseNode {
	std::vector<uint32_t> name_tokens;  // name of the variable

	std::string toString(std::vector<Token>& tokens) override;
};


// Function /////////////////////////////////////////////////////////////////////////

struct AST_FunctionDefinition : AST_BaseNode {
	uint32_t name_token;
	std::vector<uint32_t> param_nodes;
	uint32_t return_node;
	std::vector<uint32_t> modifiers_tokens;
};

struct AST_FunctionCall : AST_BaseNode {
	std::vector<uint32_t> name_tokens;
	std::vector<uint32_t> modifiers_tokens;
	// arguments will be the child expression nodes

	std::string toString(std::vector<Token>& tokens) override;
};

// Stuff that lives inside a function and can be executed
// It's children are a mix of assignments, declarations and function calls
struct AST_Statements : AST_BaseNode { };


// Type ////////////////////////////////////////////////////////////////////////////

struct AST_Type : AST_BaseNode {
	uint32_t name_token;

	std::string toString(std::vector<Token>& tokens) override;
};


//////////////////////////////////////////////////////////////////////////////////

typedef std::variant<
	// Code Spliting
	AST_SourceFile,

	// Expression
	AST_Expression,

	AST_OperatorPlusBinary,
	AST_OperatorMultiplication,

	AST_NumericLiteral,
	AST_StringLiteral,

	// Variable
	AST_VariableDeclaration,
	AST_Variable,
	AST_VariableAssignment,

	// Function
	AST_FunctionDefinition,
	AST_FunctionCall,
	AST_Statements,

	// Type
	AST_Type
> AST_Node;


struct CompilerError {
	std::string msg;

	uint32_t line;
	uint32_t column;
	std::string file_path;
};


class Parser {
public:
	std::string file_path;
	std::vector<Token> tokens;

	std::vector<AST_Node> nodes;

	std::vector<CompilerError> errors;

public:

	/* Utility functions */
	Token& getToken(uint32_t token_index);

	template<typename T>
	T* addNode(uint32_t& r_new_node_index)
	{
		r_new_node_index = nodes.size();

		return &nodes.emplace_back().emplace<T>();
	}

	template<typename T>
	T* addNode(uint32_t parent_node_index, uint32_t& r_child_node_index)
	{
		r_child_node_index = nodes.size();

		T* new_node = &nodes.emplace_back().emplace<T>();

		// link to each other
		AST_BaseNode* parent = getBaseNode(parent_node_index);
		parent->children.push_back(r_child_node_index);

		AST_BaseNode* child = getBaseNode(r_child_node_index);
		child->parent = parent_node_index;

		return new_node;
	}

	template<typename T>
	T* getNode(uint32_t node_idx)
	{
		return std::get_if<AST_Type>(&nodes[node_idx]);
	}

	AST_BaseNode* getBaseNode(uint32_t node_idx);

	void linkParentAndChild(uint32_t parent_node_index, uint32_t child_node_index);


	/* Seek Functions */

	// skip spacing and identifiers but not specified symbols to find symbol token
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
	bool skipPastCompositeName(uint32_t& token_index);

	// skip only spacing to find identifier
	bool skipToIdentifierToken(uint32_t& token_index);
	bool skipToIdentifierToken(uint32_t token_index, uint32_t& r_token_index);

	bool skipToIdentifierToken(uint32_t& token_index, std::string target_identifier);


	/* Parse Functions ******************************************************************************/
	// Parse functions advance token index just past syntax, they don't make asumptions
	// about what follows after


	/* Common */

	// parse dot separated list of identifiers
	// ex: namespace_name.class_name.property_name
	void parseCompositeName(uint32_t& token_index, std::vector<uint32_t>& r_name);

	// parse space separated list of modifiers
	// ex: modifier_0 modifier_1 modifier_2
	void parseModifiers(uint32_t& token_index, std::vector<uint32_t>& r_modifiers);


	/* Code Spliting */

	void parseFile(Lexer&& file_to_lex);


	/* Expression */

	bool parseSubExpression(uint32_t& token_index, int32_t parent_precedence,
		uint32_t& r_child_node_index);

	bool parseExpression(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);


	/* Variable */

	// a variable declaration is defined as the combination of a simple indetifier for the name and
	// a another identifier acting as the type
	bool parseVariableDeclaration(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	// assignment is defined as a complex name and the `=` sign
	bool parseVariableAssignment(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);


	/* Function */

	// parse stuff inside function body, stuff that can be executed
	/*bool parseStatements(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);*/

	bool parseFunctionImplementation(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	bool parseFunctionCall(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	bool parseStatement(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	bool parseStatements(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);


	/* Type */

	// parse the type in stuff like variable declarations, function parameters
	bool parseType(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);


	/* Check functions */

	// checks if the identifier is not followed by '.' or other symbols
	bool isSimpleName(uint32_t token_index);


	/* Error */

	void error(std::string error_mesage, uint32_t token_index);

	// "unexpected token '{token.value}' "
	void errorUnexpectedToken(std::string error_mesage, uint32_t token_index);


	/* Debug */
	void _print(uint32_t node_idx, uint32_t depth);
	void printTree();
	void printNodes(uint32_t start_index = 0, uint32_t end_index = 0xFFFF'FFFF);
};
