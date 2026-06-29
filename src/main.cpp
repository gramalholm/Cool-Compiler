#include "lexer.h"
#include "../headers/parser.h"
#include "semantic.h"
#include "../headers/codegen.h"

#include <iostream>
#include <clocale>
#include <cstdlib>

using std::cout;

int main(int argc, char* argv[]){
    std::setlocale(LC_ALL, ".UTF8");
    std::system("chcp 65001 > nul");

    string filepath = argc > 1 ? argv[1] : "teste.txt";
    SLRParser* parser = new SLRParser(filepath);
    parser->parsing();
    if (!parser->get_root()) {
        cout << "Erro: falha na analise sintatica (AST nao foi construida)" << endl;
        delete parser;
        return 1;
    }

    cout << "AST construida com sucesso" << endl;

    SemanticAnalyzer* sa = new SemanticAnalyzer();
    ASTNode* root = parser->get_root();
    Program* prog = dynamic_cast<Program*>(root);

    if (prog != nullptr) {
        sa->analyze(prog);

        // ── Geração de código Bril (apenas se a semântica passou) ──
        BrilCodeGen* cg = new BrilCodeGen();
        cg->generate(prog);
        delete cg;
    } else {
        cout << "Erro: a raiz da AST nao é Program" << endl;
    }

    delete sa;
    delete parser;
    return 0;
}