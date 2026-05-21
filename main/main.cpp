#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "ProgramParser.h"
#include "IRGenerator.h"
#include "Optimizer.h"
#include "CodeGenerator.h"

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cerr << "Usage: ./compiler program.mylang\n"; return 1; }

    std::ifstream file(argv[1]);
    if (!file.is_open()) { std::cerr << "Error: cannot open '" << argv[1] << "'\n"; return 1; }

    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    try {
        std::istringstream ss(source);
        Lexer         lexer(ss);
        SymbolTable   symTable;
        ProgramParser parser(lexer, symTable);
        auto ast = parser.parseProgram();
        std::cout << " Parsing done\n";

        IRGenerator irGen;
        TACProgram  tac = irGen.generate(ast.get());
        irGen.printTAC(tac);
        std::cout << "\n IR Generation done\n";

        Optimizer  optimizer;
        TACProgram optimized = optimizer.optimize(tac);
        std::cout << "\n═══ OPTIMIZED TAC ═══\n";
        irGen.printTAC(optimized);
        std::cout << "\n Optimization done\n";

        std::cout << "\n═══ RISC-V ASSEMBLY ═══\n";
        CodeGenerator codegen(std::cout);
        codegen.generate(optimized);
        std::cout << "\n Code Generation done\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}