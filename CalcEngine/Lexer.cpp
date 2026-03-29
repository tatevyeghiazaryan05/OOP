#include <cctype>
#include <iostream>
#include <string>
#include <istream>

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
enum class LexerState {
    Start,
    InNumber,
    InName,
    InString,
    InOperator,
    InComment,
    InEscape
};

class Lexer {
private:
    std::istream& input;
    LexerState currentState;
    std::string currentToken;
    int line;
    int column;
    int currentChar;
    bool isEscape;

    static const int STATE_COUNT = 7;
    static const int CHAR_TYPE_COUNT = 8;

    enum CharType {
        Digit,
        Letter,
        Whitespace,
        Operator_,
        Paren,
        Quote,
        Newline_,
        Other
    };
    LexerState transitionMatrix[STATE_COUNT][CHAR_TYPE_COUNT];

    void initializeTransitionMatrix();
    CharType getCharType(char c);
    void handleState(char c);
    Token createToken(TokenType type);

public:
    Lexer(std::istream& is);
    Token getNextToken();
    void reset();
};

Lexer::Lexer(std::istream& is) : input(is), currentState(LexerState::Start), line(1), column(0), currentChar(0), isEscape(false) {
    initializeTransitionMatrix();
}

void Lexer::initializeTransitionMatrix() {
    for (int i = 0; i < STATE_COUNT; i++) {
        for (int j = 0; j < CHAR_TYPE_COUNT; j++) {
            transitionMatrix[i][j] = LexerState::Start;
        }
    }
    
    transitionMatrix[static_cast<int>(LexerState::Start)][static_cast<int>(CharType::Digit)] = LexerState::InNumber;
    transitionMatrix[static_cast<int>(LexerState::Start)][static_cast<int>(CharType::Letter)] = LexerState::InName;
    transitionMatrix[static_cast<int>(LexerState::Start)][static_cast<int>(CharType::Operator_)] = LexerState::InOperator;
    transitionMatrix[static_cast<int>(LexerState::Start)][static_cast<int>(CharType::Paren)] = LexerState::Start; // Parentheses are single-character tokens
    transitionMatrix[static_cast<int>(LexerState::Start)][static_cast<int>(CharType::Quote)] = LexerState::InString;
    transitionMatrix[static_cast<int>(LexerState::Start)][static_cast<int>(CharType::Whitespace)] = LexerState::Start;
    transitionMatrix[static_cast<int>(LexerState::Start)][static_cast<int>(CharType::Newline_)] = LexerState::Start;
    
    transitionMatrix[static_cast<int>(LexerState::InNumber)][static_cast<int>(CharType::Digit)] = LexerState::InNumber;
    transitionMatrix[static_cast<int>(LexerState::InNumber)][static_cast<int>(CharType::Letter)] = LexerState::InName;
    
    transitionMatrix[static_cast<int>(LexerState::InName)][static_cast<int>(CharType::Digit)] = LexerState::InName;
    transitionMatrix[static_cast<int>(LexerState::InName)][static_cast<int>(CharType::Letter)] = LexerState::InName;
    
    transitionMatrix[static_cast<int>(LexerState::InString)][static_cast<int>(CharType::Quote)] = LexerState::Start;
    transitionMatrix[static_cast<int>(LexerState::InString)][static_cast<int>(CharType::Other)] = LexerState::InString;
    
    transitionMatrix[static_cast<int>(LexerState::InOperator)][static_cast<int>(CharType::Operator_)] = LexerState::InOperator;
}

Lexer::CharType Lexer::getCharType(char c) {
    if (isdigit(c)) return CharType::Digit;
    if (isalpha(c)) return CharType::Letter;
    if (isspace(c)) {
        if (c == '\n' || c == '\r') return CharType::Newline_;
        return CharType::Whitespace;
    }
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=') return CharType::Operator_;
    if (c == '(' || c == ')') return CharType::Paren;
    if (c == '"') return CharType::Quote;
    return CharType::Other;
}

void Lexer::handleState(char c) {
    switch (currentState) {
        case LexerState::InString:
            if (c == '\\') {
                isEscape = true;
            } else if (c == '"' && !isEscape) {
                // end string
            } else {
                isEscape = false;
                currentToken += c;
            }
            break;
        default:
            if (!isspace(c)) {
                currentToken += c;
            }
            break;
    }
}

Token Lexer::createToken(TokenType type) {
    Token token(type, currentToken, line, column - static_cast<int>(currentToken.length()));
    currentToken.clear();
    return token;
}

Token Lexer::getNextToken() {
    currentToken.clear();
    currentState = LexerState::Start;
    
    char c;
    while (input.get(c)) {
        column++;
        
        if (c == '\n') {
            line++;
            column = 0;
        }
        
        CharType charType = getCharType(c);
        LexerState newState = transitionMatrix[static_cast<int>(currentState)][static_cast<int>(charType)];
        
        if (charType == CharType::Newline_) {
            if (!currentToken.empty()) {
                input.unget();
                column--;
                break;
            }
            continue;
        }
        
        if (charType == CharType::Paren) {
            if (!currentToken.empty()) {
                input.unget();
                column--;
                break;
            } else {
                currentToken = c;
                if (c == '(') {
                    return createToken(TokenType::OpenParen);
                } else {
                    return createToken(TokenType::CloseParen);
                }
            }
        }
        
        if (newState != currentState && !currentToken.empty()) {
            input.unget();
            column--;
            break;
        }
        
        currentState = newState;
        handleState(c);
        
        if (currentState == LexerState::Start && charType == CharType::Quote && !isEscape) {
            return createToken(TokenType::Name); // String literals as names for now
        }
    }
    
    if (currentToken.empty()) {
        return Token(TokenType::EndOfExpr, "", line, column);
    }
    
    if (currentState == LexerState::InNumber) {
        return createToken(TokenType::Number);
    } else if (currentState == LexerState::InName) {
        return createToken(TokenType::Name);
    } else if (currentState == LexerState::InOperator) {
        if (currentToken == "=") {
            return createToken(TokenType::Assignment);
        }
        return createToken(TokenType::Operator);
    }
    
    return Token(TokenType::Unknown, currentToken, line, column - static_cast<int>(currentToken.length()));
}

void Lexer::reset() {
    input.clear();
    input.seekg(0);
    line = 1;
    column = 0;
    currentState = LexerState::Start;
    currentToken.clear();
    isEscape = false;
}