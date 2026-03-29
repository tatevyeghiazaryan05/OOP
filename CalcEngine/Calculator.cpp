#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include "ASTNode.h"
#include "SymboleTable.cpp"

class Calculator {
private:
    SymbolTable& symbolTable;

    double evaluateNode(ASTNode* node);

public:
    Calculator(SymbolTable& symTable);
    double calculate(std::unique_ptr<ASTNode> ast);
    void setVariable(const std::string& name, double value);
};


Calculator::Calculator(SymbolTable& symTable) : symbolTable(symTable) {}

double Calculator::evaluateNode(ASTNode* node) {
    if (!node) return 0.0;

    switch (node->type) {
        case NodeType::NumberNode: {
            auto* numNode = static_cast<NumberNode*>(node);
            return numNode->value;
        }

        case NodeType::VariableNode: {
            auto* varNode = static_cast<VariableNode*>(node);
            double value;
            if (symbolTable.getValue(varNode->name, value)) {
                return value;
            }
            throw std::runtime_error("Variable not initialized: " + varNode->name);
        }

        case NodeType::BinaryOpNode: {
            auto* binNode = static_cast<BinaryOpNode*>(node);
            double left = evaluateNode(binNode->left.get());
            double right = evaluateNode(binNode->right.get());

            switch (binNode->op) {
                case '+': return left + right;
                case '-': return left - right;
                case '*': return left * right;
                case '/':
                    if (right == 0) throw std::runtime_error("Division by zero");
                    return left / right;
                default:
                    throw std::runtime_error("Unknown binary operator");
            }
        }

        case NodeType::UnaryOpNode: {
            auto* unNode = static_cast<UnaryOpNode*>(node);
            double value = evaluateNode(unNode->operand.get());

            switch (unNode->op) {
                case '+': return value;
                case '-': return -value;
                default:
                    throw std::runtime_error("Unknown unary operator");
            }
        }

        case NodeType::AssignmentNode: {
            auto* assignNode = static_cast<AssignmentNode*>(node);
            double value = evaluateNode(assignNode->expression.get());
            symbolTable.setValue(assignNode->varName, value);
            return value;
        }

        default:
            throw std::runtime_error("Unknown node type");
    }
}

double Calculator::calculate(std::unique_ptr<ASTNode> ast) {
    if (!ast) {
        throw std::runtime_error("No expression to evaluate");
    }
    return evaluateNode(ast.get());
}

void Calculator::setVariable(const std::string& name, double value) {
    symbolTable.setValue(name, value);
}