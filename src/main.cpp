#include "lexer.h"
#include <iostream>
using std::cout;

int main(){
    Lexer lexer;
    while(true){
        auto token = lexer.scan();
        if(token->get_type() == lextoken_type::eof_token){
            cout << "Fim do arquivo lido na linha: "<< lexer.get_line_num() << endl;
            break;
        }
        else if(token->get_type() == lextoken_type::error_token){
            cout << "Token de erro encontrado na linha " << lexer.get_line_num() << "." << endl;
        }

        /*
            Como estamos usando polimorfismo, estou usando o dynamic_cast
            para ver qual o tipo do token e ai printar ou uma string ou um num.
        */
        if(auto s = dynamic_cast<StrToken*>(token.get())){
            if(s->get_type() == lextoken_type::identifier_token){
                cout << "Token identificador: " << s->get_token_str() << endl;
            }
            else{
                cout << "Token string:  " << s->get_token_str() << endl;
            }
        } else if( auto n = dynamic_cast<numToken*>(token.get())){
            cout << "Token tipo numero: " << n->get_token_num() << endl;
        } else {
            cout << "Operador ou pontuacao: " << lexer.type_to_string(token->get_type()) << endl;
    }
    }
}