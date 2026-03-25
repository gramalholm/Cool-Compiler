#include "lexer.h"
#include <string>

Lexer::Lexer() {
    str_table["class"] = lextoken_type::class_token;
    str_table["then"] = lextoken_type::then_token;
    str_table["fi"] = lextoken_type::fi_token;
    str_table["in_kw"] = lextoken_type::in_kw_token;
    str_table["inherits"] = lextoken_type::inherits_token;
    str_table["isvoid"] = lextoken_type::isvoid_token;
    str_table["let"] = lextoken_type::let_token;
    str_table["loop"] = lextoken_type::loop_token;
    str_table["case"] = lextoken_type::case_token;
    str_table["esac"] = lextoken_type::esac_token;
    str_table["new"] = lextoken_type::new_token;
    str_table["of"] = lextoken_type::of_token;
    str_table["not"] = lextoken_type::not_token;
    str_table["pool"] = lextoken_type::pool_token;
    str_table["if"] = lextoken_type::if_token;
    str_table["else"] = lextoken_type::else_token;
    str_table["while"] = lextoken_type::while_token;
    str_table["true"] = lextoken_type::true_token;
    str_table["false"] = lextoken_type::false_token;
}

Lextoken Lexer::scan() {
    // Implementação do método de análise léxica para ler os caracteres e criar os tokens.
    // Este método deve ser implementado para ler a entrada, identificar os tokens e retornar o token correspondente.
    // O código para esta função não foi fornecido, mas deve incluir a lógica para lidar com diferentes tipos de tokens, como palavras reservadas, identificadores, números, operadores, etc.
    return Lextoken(lextoken_type::eof_token); // Retorna um token EOF como exemplo.
}

Lextoken::Lextoken(lextoken_type t): type(t) {}

StrToken::StrToken(lextoken_type t, std::string str): Lextoken(t), token_str(str) {}

numToken::numToken(lextoken_type t, int num): Lextoken(t), token_num(num) {}