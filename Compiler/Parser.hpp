#pragma once

// Standard
#include <variant>
#include <typeinfo>
#include <unordered_map>

// Mine
#include "Lexer/Lexer.hpp"


typedef uint32_t AST_NodeIndex;


struct SourceCodePosition {
	uint32_t line;
	uint32_t column;

	void operator=(const Token&);
};

// Base class for all abstract syntax tree nodes, never used on its own
struct AST_BaseNode {
	uint32_t parent = 0xFFFF'FFFF;
	std::vector<uint32_t> children;

	SourceCodePosition start_pos;
	SourceCodePosition end_pos;

	virtual std::string toString() {
		return "Node";
	};
};


// Code Spliting ////////////////////////////////////////////////////////////////////

struct AST_SourceFile : AST_BaseNode {
	std::string toString() override;
};


// Expression ///////////////////////////////////////////////////////////////////////

// Expression used in assignment or function call
struct AST_Expression : AST_BaseNode {
	std::string toString() override {
		return "Expression";
	};
};

struct AST_BinaryOperator : AST_BaseNode {
	Token token;

	void assign(const Token& new_token)
	{
		this->token = new_token;
		this->start_pos = new_token;
		this->end_pos.line = new_token.line;
		this->end_pos.column = new_token.column + 1;
	}

	std::string toString() override {
		return token.value;
	};
};

struct AST_Literal : AST_BaseNode {
	Token token;

	std::string toString() override;
};


// Variable /////////////////////////////////////////////////////////////////////////

struct AST_VariableDeclaration : AST_BaseNode {
	Token name_token;  // name of the variable

	AST_NodeIndex type;
	AST_NodeIndex default_expr;

public:
	std::string toString() override;
};

struct AST_Variable : AST_BaseNode {
	std::vector<Token> name_tokens;  // name of the variable

	std::string toString() override;
};

struct AST_VariableAssignment : AST_BaseNode {
	std::vector<Token> name_tokens;  // name of the variable

	std::string toString() override;
};


// Function /////////////////////////////////////////////////////////////////////////

struct AST_FunctionDefinition : AST_BaseNode {
	uint32_t name_token;
	std::vector<uint32_t> param_nodes;
	uint32_t return_node;
	std::vector<uint32_t> modifiers_tokens;
};

struct AST_FunctionCall : AST_BaseNode {
	std::vector<Token> name_tokens;
	std::vector<Token> modifiers_tokens;
	// arguments will be the child expression nodes

	std::string toString() override;
};

// Stuff that lives inside a function and can be executed
// It's children are a mix of assignments, declarations and function calls
struct AST_Statements : AST_BaseNode {

	// variable declarations
};


// Type ////////////////////////////////////////////////////////////////////////////

struct AST_Type : AST_BaseNode {
	Token name;

	std::string toString() override;
};


//////////////////////////////////////////////////////////////////////////////////

typedef std::variant<
	// Code Spliting
	AST_SourceFile,

	// Expression
	AST_Expression,

	AST_BinaryOperator,
	AST_Literal,

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


struct CompilerMessage {
	std::string msg;

	// @HERE
	uint32_t line;
	uint32_t column;
};

struct PrintAST_TreeSettings {
	bool show_node_index = false;
	bool show_source_ranges = false;
};


class Parser {
public:
	Lexer lexer;
	std::vector<AST_Node> nodes;

	std::vector<CompilerMessage> errors;

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
		return &std::get<T>(nodes[node_idx]);
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
	void parseCompositeName(uint32_t& token_index, std::vector<Token>& r_name);

	// parse space separated list of modifiers
	// ex: modifier_0 modifier_1 modifier_2
	void parseModifiers(uint32_t& token_index, std::vector<Token>& r_modifiers);


	/* Code Spliting */

	void parseFile(std::vector<uint8_t>& file_bytes, std::string file_path);


	/* Expression */

	bool _parseExpression(uint32_t& token_index, int32_t parent_precedence,
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
	void _printTree(uint32_t node_idx, uint32_t depth, PrintAST_TreeSettings&);
	void printTree(PrintAST_TreeSettings settings = PrintAST_TreeSettings());
	void printNodes(uint32_t start_index = 0, uint32_t end_index = 0xFFFF'FFFF);
};


typedef uint32_t TypeNodeIndex;


struct VariableDeclaration {
	uint32_t node;

	std::unordered_map<std::string, uint32_t> children;
};

struct TypeNode {
	TypeNodeIndex parent;
	std::unordered_map<std::string, TypeNodeIndex> children;

	// the AST_Node that triggered this type declaration to facilitate code navigation
	AST_NodeIndex ast_node;
	std::string name;
};

class TypeStuff {
public:
	Parser parser;

	std::vector<VariableDeclaration> variables;
	std::vector<TypeNode> types;

	TypeNode* i32_type;
	TypeNode* f32_type;

public:

	TypeNode& addType(TypeNodeIndex parent_node, AST_NodeIndex ast_node, const std::string& name)
	{
		TypeNode& parent = types[parent_node];
		parent.children.insert({ name, this->types.size() });

		TypeNode& new_type = this->types.emplace_back();
		new_type.parent = parent_node;
		new_type.ast_node = ast_node;
		new_type.name = name;

		return new_type;
	}

	TypeNode& addType(TypeNodeIndex parent_node, AST_NodeIndex ast_node, const char* name)
	{
		std::string str = name;
		return addType(parent_node, ast_node, str);
	}

	std::string toString(TypeNode* type)
	{
		std::string result = type->name;
		TypeNodeIndex type_node_index = type->parent;

		while (type_node_index != 0xFFFF'FFFF) {

			type = &types[type_node_index];

			result.insert(0, ".");
			result.insert(0, type->name);
		}

		return result;
	}

	bool isCompatible(TypeNode* left, TypeNode* right)
	{
		TypeNode* a = left;
		TypeNode* b = right;

		while (true) {

			if (a->name != b->name) {
				return false;
			}

			if (a->parent == 0xFFFF'FFFF || b->parent == 0xFFFF'FFFF) {
				return a->parent == b->parent;
			}

			a = &types[a->parent];
			b = &types[b->parent];
		}

		throw std::exception();
		return true;
	}

	/*TypeNode* findType(TypeNode* context, AST_NodeIndex ast_type)
	{
		 
	}*/

	void errorBinaryOperatorIncompatibleTypes(AST_BinaryOperator binary_op, TypeNode* a, TypeNode* b)
	{
		CompilerMessage& new_error = parser.errors.emplace_back();
		new_error.msg = "Incompatible operand types: \n";
		new_error.msg += "  " + toString(a) + "\n";
		new_error.msg += "  " + toString(b) + "\n";
		new_error.msg += "For operator: " + binary_op.toString() + "\n";
		new_error.line = binary_op.start_pos.line;
		new_error.column = binary_op.start_pos.column;
	}

	bool typeCheckExpression(AST_NodeIndex ast_node_index, TypeNode*& r_type)
	{
		AST_Node& ast_node = parser.nodes[ast_node_index];

		if (std::holds_alternative<AST_Literal>(ast_node)) {

			auto& literal = std::get<AST_Literal>(ast_node);

			switch (literal.token.type) {
			case TokenTypes::i32: {
				r_type = i32_type;
				return true;
			}
			case TokenTypes::f32: {
				r_type = f32_type;
				return true;
			}
			default:
				throw;
			}
		}
		// Type check binary operand
		else if (std::holds_alternative<AST_BinaryOperator>(ast_node)) {

			auto& binary_op = std::get<AST_BinaryOperator>(ast_node);
			
			TypeNode* left_type;
			if (typeCheckExpression(binary_op.children[0], left_type) == false) {
				return false;
			}

			TypeNode* right_type;
			if (typeCheckExpression(binary_op.children[1], right_type) == false) {
				return false;
			}

			if (isCompatible(left_type, right_type) == false) {
				errorBinaryOperatorIncompatibleTypes(binary_op, left_type, right_type);
			}

			r_type = left_type;
			return true;
		}
		else {
			throw;
		}
	}

	void typeCheckFile()
	{
		{
			auto& root_var = variables.emplace_back();
			root_var.node = 0;

			auto& root_type = types.emplace_back();
			root_type.parent = 0xFFFF'FFFF;

			i32_type = &addType(0, 0xFFFF'FFFF, "i32");
			f32_type = &addType(0, 0xFFFF'FFFF, "f32");
		}

		std::vector<AST_NodeIndex> now_nodes;
		std::vector<AST_NodeIndex> next_nodes;

		TypeNode* context_type = &types[0];

		while (next_nodes.size()) {

			for (auto now_node : now_nodes) {

				AST_Node& now = parser.nodes[now_node];

				if (std::holds_alternative<AST_VariableDeclaration>(now)) {

					auto var_decl = parser.getNode<AST_VariableDeclaration>(now_node);


					if (var_decl->default_expr != 0xFFFF'FFFF) {

						auto default_expr = parser.getNode<AST_Expression>(var_decl->default_expr);
					}
				}
			}
		}
	}
};