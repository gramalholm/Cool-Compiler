#include <string>
#include "token.h"

/*
ta faltando token ainda eu acho (revisar)

tokens devem ser: palavras reservadas, identificadores, literais (string e int), 
simbolos (operadores e pontuacao), EOF, error, comentarios e whitespace (se for necessario para o parser, caso contrario pode ser ignorado)
*/
Lextoken::Lextoken(lextoken_type t, std::string str, int line)
    : type(t), token_str(str), line_num(line) {}


lextoken_type Lextoken::get_type() const {
    return type;
}

const std::string& Lextoken::get_token_str() const {
    return token_str;
}

int Lextoken::get_line_num() const {
    return line_num;
}

