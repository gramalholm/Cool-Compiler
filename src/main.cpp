#include "lexer.h"
#include "../headers/parser.h"
#include <iostream>
using std::cout;

int main(int argc, char* argv[]){
    string filepath = argc > 1 ? argv[1] : "teste.txt";
    SLRParser* parser = new SLRParser(filepath);
    parser->parsing();
    if (parser->get_root()) {
        cout << "AST construida com sucesso" << endl;
    }
    delete parser;
    return 0;
}