#pragma once

// Standard
#include <variant>
#include <typeinfo>
#include <unordered_map>
#include <map>

// Mine
#include "../Lexer/Lexer.hpp"


typedef uint32_t AST_NodeIndex;
typedef uint32_t DeclNodeIndex;

const AST_NodeIndex ast_invalid_idx = 0xFFFF'FFFF;


// Base class for all abstract syntax tree nodes, never used on its own
struct AST_BaseNode {
	AST_NodeIndex parent = 0xFFFF'FFFF;
	std::vector<AST_NodeIndex> children;

	CodeSelection selection;

	void setStart(const Token&);
	void setEnd(const Token&);

	virtual std::string toString() {
		return "AST_Node"s;
	};
};

struct AST_Declaration {
	CodeSelection name_selection;
};


// Code Spliting ////////////////////////////////////////////////////////////////////

struct AST_Root : AST_BaseNode {
	std::string toString() override;
};

struct AST_SourceFile : AST_BaseNode {
	DeclNodeIndex decl_node;

	std::string toString() override;
};


// Expression ///////////////////////////////////////////////////////////////////////

// Expression used in assignment or function call
struct AST_Expression : AST_BaseNode {
	std::string toString() override {
		return "Expression"s;
	};
};

struct AST_BinaryOperator : AST_BaseNode {
	Token token;

	void assign(const Token& new_token)
	{
		this->token = new_token;
		this->selection = new_token;
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

struct AST_VariableDeclaration : AST_BaseNode, AST_Declaration {
	Token name;  // name of the variable

	AST_NodeIndex type;
	AST_NodeIndex default_expr;

	DeclNodeIndex decl_node;

public:
	std::string toString() override;
};

struct AST_Variable : AST_BaseNode {
	std::vector<Token> address;  // name of the variable

	std::string toString() override;
};

struct AST_VariableAssignment : AST_BaseNode {
	std::vector<Token> address;  // name of the variable

	DeclNodeIndex decl_node;

	std::string toString() override;
};


// Function /////////////////////////////////////////////////////////////////////////

struct AST_FunctionImplementation : AST_BaseNode, AST_Declaration {
	std::vector<Token> name;
	std::vector<AST_NodeIndex> params;
	AST_NodeIndex returns;
	std::vector<Token> modifiers;
	AST_NodeIndex statements;

	DeclNodeIndex decl_node;

public:
	std::string toString() override;
};

struct AST_FunctionCall : AST_BaseNode {
	std::vector<Token> address;
	std::vector<Token> modifiers_tokens;
	// arguments will be the child expression nodes

	std::string toString() override;
};

// Stuff that lives inside a function and can be executed
// It's children are a mix of assignments, declarations and function calls
struct AST_Statements : AST_BaseNode {
	std::string toString() override;
};


// Type ////////////////////////////////////////////////////////////////////////////

struct AST_Type : AST_BaseNode {
	Token name;

	std::string toString() override;
};

struct AST_TypeDeclaration : AST_BaseNode {
	Token name;
};


//////////////////////////////////////////////////////////////////////////////////

typedef std::variant<
	AST_Root,

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
	AST_FunctionImplementation,
	AST_FunctionCall,
	AST_Statements,

	// Type
	AST_Type,
	AST_TypeDeclaration
> AST_Node;

enum class MessageSeverity : uint32_t {
	Message,
	Warning,
	Error
};

struct MessageRow {
	std::string text;
	CodeSelection selection;
};

struct CompilerMessage {
	std::vector<MessageRow> rows;
	MessageSeverity severity;
};

struct PrintAST_TreeSettings {
	bool show_node_index = false;
	bool show_code_selections = false;
};

std::string getAdressName(std::vector<Token>& tokens);


class Parser {
public:
	Lexer lexer;
	std::vector<AST_Node> nodes;

	TokenIndex token_i;
	TokenIndex unexpected_idx;

	std::vector<CompilerMessage> messages;

public:
	void init();


	/* Utility functions */

	Token& getToken(uint32_t token_index);
	Token& getToken();

	template<typename T>
	T* getNode(AST_NodeIndex ast_node_idx)
	{
		return &std::get<T>(nodes[ast_node_idx]);
	}

	AST_BaseNode* getBaseNode(AST_NodeIndex ast_node_idx);
	CodeSelection getNodeSelection(AST_NodeIndex ast_node_idx);

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

	void linkParentAndChild(uint32_t parent_node_index, uint32_t child_node_index);


	/* New Skip Functions */

	void advanceToNextToken();

	bool skipSpacing();

	bool skipToIdentifier();

	bool skipToSymbol(std::string symbol);

	bool skipToClosingSymbol(std::string starting_symbol, std::string closing_symbol);


	/* Seek Functions */

	// skip spacing and identifiers but not specified symbols to find symbol token
	bool seekToSymbolToken(uint32_t& token_index, std::string symbol_token,
		std::vector<std::string> not_allowed_symbols, bool allow_identifier = true);

	// skip spacing and identifiers to reach for symbol
	bool seekToSymbolToken(uint32_t& token_index, std::string symbol_token);


	/* Skip functions */

	bool skipSpacing(TokenIndex& token_index);

	// skip only spacing to find symbol
	bool skipToSymbolToken(uint32_t& token_index, std::string target_symbol);
	bool skipToSymbolToken(uint32_t token_index, std::string target_symbol, uint32_t& r_token_index);

	bool skipToExpressionSymbolToken(uint32_t token_index, uint32_t& r_token_index);
	bool skipToNumberToken(uint32_t token_index, uint32_t& r_token_index);
	bool skipToStringToken(uint32_t token_index, uint32_t& r_token_index);

	// skip anything to reach closing end symbol,
	// token index must start AT start symbol token 
	bool skipToClosingSymbolToken(uint32_t& token_index,
		std::string start_symbol_token, std::string end_symbol_token);

	// skip spacing and identifiers
	bool skipPastIdentifiers(uint32_t& token_index);
	bool skipPastCompositeName(uint32_t& token_index);

	// skip only spacing to find identifier
	bool skipToIdentifierToken(uint32_t& token_index);
	bool skipToIdentifierToken(uint32_t token_index, uint32_t& r_token_index);
	bool skipToIdentifierToken(uint32_t& token_index, std::string target_identifier);
	bool skipToIdentifierToken(uint32_t& token_index, std::string target_identifier, uint32_t& r_token_index);


	/* Check functions */

	// checks if the identifier is not followed by '.' or other symbols
	bool isSimpleName(uint32_t token_index);

	bool isAtAdress();


	/* Parse Functions ******************************************************************************/
	// Parse functions advance token index just past syntax, they don't make asumptions
	// about what follows after


	/* Common */

	// parse dot separated list of identifiers
	// ex: namespace_name.class_name.property_name
	void parseCompositeName(uint32_t& token_index, std::vector<Token>& r_name);
	void parseAdress(std::vector<Token>& r_adress);

	// parse space separated list of modifiers
	// ex: modifier_0 modifier_1 modifier_2
	void parseModifiers(uint32_t& token_index, std::vector<Token>& r_modifiers);


	/* Expression */

	bool _parseExpression(uint32_t& token_index, int32_t parent_precedence,
		uint32_t& r_child_node_index);

	bool parseExpression(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);


	/* Variable */

	// a variable declaration is defined as the combination of a simple indetifier for the name and
	// a another identifier acting as the type
	AST_NodeIndex parseVariableDeclaration(AST_NodeIndex parent_node_index);

	// assignment is defined as a complex name and the `=` sign
	bool parseVariableAssignment(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);


	/* Function */

	bool parseFunctionImplementation(AST_NodeIndex parent_node,	AST_NodeIndex& r_func_impl);

	bool parseFunctionCall(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	// @HERE
	bool parseStatement(AST_NodeIndex ast_parent, uint32_t& token_index,
		AST_NodeIndex& r_statement);

	AST_NodeIndex parseStatements(AST_NodeIndex ast_parent);


	/* Type */

	// parse the ast_type in stuff like variable declarations, function parameters
	bool parseType(uint32_t parent_node_index, uint32_t& token_index,
		uint32_t& r_child_node_index);

	// bool parseTypeDeclaration();


	/* Source File */

	AST_NodeIndex parseDeclaration(AST_NodeIndex ast_parent);

	AST_NodeIndex parseSourceFile();


	/* Error */

	void pushError(std::string error_mesage, TokenIndex token_index);
	void pushError(std::string error_mesage);

	// "unexpected token '{token.value}' "
	void errorUnexpectedToken(std::string error_mesage, Token& unexpected_token);
	void errorUnexpectedToken(std::string error_mesage, TokenIndex unexpected_token);
	void errorUnexpectedToken(std::string error_mesage);



	/* Debug */
	void _printTree(uint32_t node_idx, uint32_t depth, PrintAST_TreeSettings&);
	void printAST(PrintAST_TreeSettings settings = PrintAST_TreeSettings());
	void printNodes(uint32_t start_index = 0, uint32_t end_index = 0xFFFF'FFFF);
};


 

class TypeChecker {
public:
	

public:

	/*std::string toString(TypeNode* type)
	{
		std::string result = type->name;
		TypeNodeIndex type_node_index = type->parent;

		while (type_node_index != 0xFFFF'FFFF) {

			type = &types[type_node_index];

			result.insert(0, ".");
			result.insert(0, type->name);
		}

		return result;
	}*/

	/*bool isCompatible(TypeNodeIndex left, TypeNodeIndex right)
	{
		TypeNode* a = &types[left];
		TypeNode* b = &types[right];

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
	}*/

	//uint32_t findType(Parser& parser, DeclarationNode* scope, AST_NodeIndex ast_type_idx)
	//{
	//	auto* ast_type = parser.getNode<AST_Type>(ast_type_idx);

	//	// normal type
	//	if (ast_type->children.size() == 0) {

	//		std::string& type_name = ast_type->name.value;

	//		auto iter = scope->children.find(type_name);

	//		if (iter != scope->children.end()) {

	//			return iter->second;
	//		}
	//		else {
	//			if (scope->parent != 0xFFFF'FFFF) {
	//				return findType(parser, &parser.decls[scope->parent], ast_type_idx);
	//			}
	//			else {
	//				return 0xFFFF'FFFF;
	//			}
	//		}
	//	}
	//	// template
	//	else {
	//		throw;
	//		return 0xFFFF'FFFF;
	//	}
	//}

	//void errorBinaryOperatorIncompatibleTypes(AST_BinaryOperator binary_op, TypeNode* a, TypeNode* b)
	//{
	//	CompilerMessage& new_error = parser.messages.emplace_back();
	//	new_error.msg = "Incompatible operand types: \n";
	//	new_error.msg += "  " + toString(a) + "\n";
	//	new_error.msg += "  " + toString(b) + "\n";
	//	new_error.msg += "For operator: " + binary_op.toString() + "\n";
	//	new_error.line = binary_op.start_pos.line;
	//	new_error.column = binary_op.start_pos.column;
	//}

	//bool _typeCheckExpression(Parser& parser, DeclarationNode* scope, AST_NodeIndex ast_node_index)
	//{
	//	AST_Node& ast_node = parser.nodes[ast_node_index];

	//	if (std::holds_alternative<AST_Literal>(ast_node)) {

	//		auto& literal = std::get<AST_Literal>(ast_node);

	//		switch (literal.token.type) {
	//		case TokenTypes::i32: {
	//			r_type = i32_type;
	//			return true;
	//		}
	//		case TokenTypes::f32: {
	//			r_type = f32_type;
	//			return true;
	//		}
	//		default:
	//			throw;
	//		}
	//	}
	//	// Type check binary operand
	//	else if (std::holds_alternative<AST_BinaryOperator>(ast_node)) {

	//		auto& binary_op = std::get<AST_BinaryOperator>(ast_node);
	//		
	//		uint32_t left_type;
	//		if (_typeCheckExpression(binary_op.children[0], left_type) == false) {
	//			return false;
	//		}

	//		uint32_t right_type;
	//		if (_typeCheckExpression(binary_op.children[1], right_type) == false) {
	//			return false;
	//		}

	//		/*if (isCompatible(left_type, right_type) == false) {
	//			errorBinaryOperatorIncompatibleTypes(binary_op,
	//				&types[left_type],
	//				&types[right_type]
	//			);
	//		}*/

	//		r_type = left_type;
	//		return true;
	//	}
	//	else {
	//		throw;
	//	}
	//}

	//bool typeCheckExpression(Parser& parser, DeclarationNode* scope, AST_NodeIndex ast_expression_idx)
	//{
	//	auto* ast_expression = parser.getNode<AST_Expression>(ast_expression_idx);

	//	return _typeCheckExpression(parser, scope, ast_expression->children[0]);
	//}

	//bool typeCheckStatements(Parser& parser, DeclarationNode* statements_scope)
	//{
	//	auto* statements = parser.getNode<AST_Statements>(statements_scope->ast_node);

	//	for (AST_NodeIndex child_node_index : statements->children) {

	//		AST_Node& child = parser.nodes[child_node_index];

	//		if (std::holds_alternative<AST_VariableDeclaration>(child)) {

	//			auto var_decl = std::get_if<AST_VariableDeclaration>(&child);
	//			var_decl->declared_type = findType(parser, statements_scope, var_decl->type);

	//			if (var_decl->default_expr != 0xFFFF'FFFF) {

	//				auto default_expr = parser.getNode<AST_Expression>(var_decl->default_expr);

	//				if (typeCheckExpression(var_decl->default_expr, var_decl->assigned_type)) {
	//					// it's fine
	//				}
	//				else {

	//				}
	//			}
	//		}
	//	}
	//}

	//bool typeCheckFile(Parser& parser)
	//{
	//	auto* ast_file = parser.getNode<AST_SourceFile>(0);

	//	for (AST_NodeIndex child_node_index : ast_file->children) {

	//		AST_Node& child = parser.nodes[child_node_index];

	//		if (std::holds_alternative<AST_Statements>(child)) {

	//			auto* statements = parser.getNode<AST_Statements>(child_node_index);
	//			typeCheckStatements(context, statements);
	//		}
	//	}
	//}
};