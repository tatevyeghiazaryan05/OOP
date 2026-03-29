#include <iostream>
#include <memory>
#include <stack>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>


enum class TokenType {
    EndOfExpr,
    Number,
    Name,
    Operator,
    OpenParen,
    CloseParen,
    Assignment,
    Unknown,
    Error
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;

    Token(TokenType t = TokenType::Unknown, const std::string& v = "", int l = 0, int c = 0)
        : type(t), value(v), line(l), column(c) {}
};

enum class NodeType {
    NumberNode,
    VariableNode,
    BinaryOpNode,
    UnaryOpNode,
    AssignmentNode
};

class ASTNode {
public:
    NodeType type;
    virtual ~ASTNode() = default;
};

struct NumberNode : public ASTNode {
    double value;
    NumberNode(double v) : value(v) { type = NodeType::NumberNode; }
};

struct VariableNode : public ASTNode {
    std::string name;
    VariableNode(const std::string& n) : name(n) {
        type = NodeType::VariableNode;
    }
};

struct BinaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryOpNode(char o, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(o), left(std::move(l)), right(std::move(r)) {
        type = NodeType::BinaryOpNode;
    }
};

struct UnaryOpNode : public ASTNode {
    char op;
    std::unique_ptr<ASTNode> operand;

    UnaryOpNode(char o, std::unique_ptr<ASTNode> node)
        : op(o), operand(std::move(node)) {
        type = NodeType::UnaryOpNode;
    }
};

struct AssignmentNode : public ASTNode {
    std::string varName;
    std::unique_ptr<ASTNode> expression;

    AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> expr)
        : varName(name), expression(std::move(expr)) {
        type = NodeType::AssignmentNode;
    }
};

class SymbolTable {
public:
    bool exists(const std::string&) const { return true; }
    void addSymbol(const std::string&, double) {}
};

class Lexer {
public:
    Token getNextToken(); 
};


enum class ParserState {
    Start,
    Operand,
    Operator,
    Assignment,
    Error
};

class Parser {
private:
    Lexer& lexer;
    SymbolTable& symbolTable;
    ParserState currentState;
    std::stack<std::unique_ptr<ASTNode>> nodeStack;
    std::stack<char> operatorStack;
    Token currentToken;
    
    static const int STATE_COUNT = 4;
    static const int TOKEN_TYPE_COUNT = 7;
    
    ParserState transitionMatrix[STATE_COUNT][TOKEN_TYPE_COUNT];
    void initializeTransitionMatrix();
    int getTokenTypeIndex(TokenType type);
    void processToken();
    int getOperatorPrecedence(char op);
    void applyOperator();
    
public:
    Parser(Lexer& lex, SymbolTable& symTable);
    std::unique_ptr<ASTNode> parse();
    void reset();
};

Parser::Parser(Lexer& lex, SymbolTable& symTable)
    : lexer(lex), symbolTable(symTable), currentState(ParserState::Start) {
    initializeTransitionMatrix();
}

void Parser::initializeTransitionMatrix() {
    for (int i = 0; i < STATE_COUNT; i++) {
        for (int j = 0; j < TOKEN_TYPE_COUNT; j++) {
            transitionMatrix[i][j] = ParserState::Error;
        }
    }

    transitionMatrix[0][1] = ParserState::Operand;
    transitionMatrix[0][2] = ParserState::Operand;
    transitionMatrix[0][4] = ParserState::Operator;
    transitionMatrix[0][3] = ParserState::Operator;

    transitionMatrix[1][3] = ParserState::Operator;
    transitionMatrix[1][5] = ParserState::Operator;
    transitionMatrix[1][0] = ParserState::Start;
    transitionMatrix[1][6] = ParserState::Assignment;

    transitionMatrix[2][1] = ParserState::Operand;
    transitionMatrix[2][2] = ParserState::Operand;
    transitionMatrix[2][4] = ParserState::Operator;
    transitionMatrix[2][3] = ParserState::Operator;

    transitionMatrix[3][1] = ParserState::Operand;
    transitionMatrix[3][2] = ParserState::Operand;
    transitionMatrix[3][4] = ParserState::Operator;
}

int Parser::getTokenTypeIndex(TokenType type) {
    switch (type) {
        case TokenType::EndOfExpr: return 0;
        case TokenType::Number: return 1;
        case TokenType::Name: return 2;
        case TokenType::Operator: return 3;
        case TokenType::OpenParen: return 4;
        case TokenType::CloseParen: return 5;
        case TokenType::Assignment: return 6;
        default: return 0;
    }
}

int Parser::getOperatorPrecedence(char op) {
    switch (op) {
        case '=': return 0;
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        default: return 0;
    }
}

void Parser::applyOperator() {
    if (operatorStack.empty()) return;

    char op = operatorStack.top();
    operatorStack.pop();

    if (op == '=') {
        auto expr = std::move(nodeStack.top()); nodeStack.pop();
        auto varNode = static_cast<VariableNode*>(nodeStack.top().get());
        std::string varName = varNode->name;
        nodeStack.pop();

        nodeStack.push(std::make_unique<AssignmentNode>(varName, std::move(expr)));
        return;
    }

    if (nodeStack.size() == 1) {
        auto right = std::move(nodeStack.top()); nodeStack.pop();
        nodeStack.push(std::make_unique<UnaryOpNode>(op, std::move(right)));
        return;
    }

    auto right = std::move(nodeStack.top()); nodeStack.pop();
    auto left = std::move(nodeStack.top()); nodeStack.pop();

    nodeStack.push(std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right)));
}

void Parser::processToken() {
    switch (currentToken.type) {
        case TokenType::Number:
            nodeStack.push(std::make_unique<NumberNode>(std::stod(currentToken.value)));
            break;

        case TokenType::Name:
            nodeStack.push(std::make_unique<VariableNode>(currentToken.value));
            break;

        case TokenType::Operator: {
            char op = currentToken.value[0];

            while (!operatorStack.empty() &&
                   getOperatorPrecedence(operatorStack.top()) >= getOperatorPrecedence(op)) {
                applyOperator();
            }

            operatorStack.push(op);
            break;
        }

        case TokenType::OpenParen:
            operatorStack.push('(');
            break;

        case TokenType::CloseParen:
            while (!operatorStack.empty() && operatorStack.top() != '(') {
                applyOperator();
            }
            operatorStack.pop();
            break;

        case TokenType::Assignment:
            operatorStack.push('=');
            break;

        default:
            break;
    }
}

std::unique_ptr<ASTNode> Parser::parse() {
    currentState = ParserState::Start;

    while (true) {
        currentToken = lexer.getNextToken();

        if (currentToken.type == TokenType::EndOfExpr) break;

        processToken();
    }

    while (!operatorStack.empty()) {
        applyOperator();
    }

    return std::move(nodeStack.top());
}

void Parser::reset() {
    while (!nodeStack.empty()) nodeStack.pop();
    while (!operatorStack.empty()) operatorStack.pop();
}