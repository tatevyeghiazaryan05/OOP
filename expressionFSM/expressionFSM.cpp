#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum TokenType { 
    VARIABLE,
    NUMBER ,
    PLUS,
    MINUS,
    MUL,
    DIV,
    LPAREN,
    RPAREN,
};

struct Token
{
    TokenType type;
    string  value;
};

bool isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')');
}

vector<string> Lexer(string exp){
    vector <string> s;
    for(int i=0; i< exp.length(); i++){
        if (isdigit(exp[i]))
        {
            string num;
            while(i < exp.length() && isdigit(exp[i])){
                num += exp[i];
                i++;
            }
            s.push_back(num);
            i--;
        }
        else if (isalpha(exp[i]))
        {
            string var(1,exp[i]);
            s.push_back(var);           
        }
        else if (isOperator(exp[i])){
            string op(1,exp[i]); 
            s.push_back(op);
        }
    }
    return (s);
}

vector <Token> Tokenizer(vector<string> lexemes)
{
    vector <Token> tokens;
    for (int i=0; i < lexemes.size(); i++){
        string s = lexemes[i];

        if (isdigit(s[0]))
            tokens.push_back({NUMBER, s});
        else if (isalpha(s[0]))
            tokens.push_back({VARIABLE, s});
        else if (lexemes[i] == "+")
            tokens.push_back({PLUS, s});
        else if (s == "-")
            tokens.push_back({MINUS, s});
        else if (s == "*")
            tokens.push_back({MUL, s});
        else if (s == "/")
            tokens.push_back({DIV, s});
        else if (s == "(")
            tokens.push_back({LPAREN, s});
        else if (s == ")")
            tokens.push_back({RPAREN, s});
    }
    return tokens;
}

int main(){
    string s = "(5+7) * 8";
    vector<string> l = Lexer(s);
    vector<Token> t = Tokenizer(l);
    cout << "t: ";
    for (int i = 0; i < t.size(); i++)
    {
        cout << t[i].type << " : " << t[i].value << endl;
    }
    cout << endl;
}