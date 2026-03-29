#ifndef LEXER_H
#define LEXER_H
#include <string>
#include <unordered_map>
using namespace std;

enum class lextoken_type {
    //palavras reservadas
    class_token, 
    if_token, 
    then_token, 
    else_token, 
    fi_token,
    in_kw_token, 
    inherits_token,
    isvoid_token, 
    let_token, 
    loop_token, 
    case_token,
    while_token,
    esac_token, 
    new_token, 
    of_token, 
    not_token,
    pool_token,
    true_token,
    false_token,

    //identificadores, numeros e strings
    identifier_token,
    int_token,
    string_token,
    bool_token,

    //operadores e pontuacao
    plus_token, // +
    minus_token, // -
    star_token, // *
    slash_token, // /
    dot_token, // .
    at_token, // @

    assign_token, // <-
    equal_token, // =
    greater_token, // >
    less_token, // <
    greater_equal_token, // >=
    less_equal_token, // <=
    not_equal_token, // !=

    semicolon_token, // ;
    dot_token, // .
    at_token, // @
    comma_token, // ,
    tilde_token, // ~
    lparen_token, // (
    rparen_token, // )
    lbrace_token, // {
    rbrace_token, // }

    // ERROR e END OF FILE
    error_token, // token de erro para caracteres não reconhecidos
    eof_token, // token de fim de arquivo

    //tokens para tipos
    typeID_token, //começa com letra maiúscula
    objectID_token, //começa com letra minúscula
};
//operadores e pontuacao: + - ; ( ) { } =
class Lextoken{
    private:
        lextoken_type type;
    public:
        // Ler os caracteres e no final criar o token com tipo, lexema e linha.
        Lextoken(lextoken_type t);
        lextoken_type get_type() const;
        virtual ~Lextoken() = default;
};

//palavras reservadas, identificadores
class StrToken : public Lextoken {
    private:
        std::string token_str;
    public:
        StrToken(lextoken_type t, std::string str);
        const std::string& get_token_str() const;
};

//valores numericos
class numToken : public Lextoken {
    private:
        int token_num;
    public:
        numToken(lextoken_type t, int num);
        int get_token_num() const;
};

class Lexer {
    private:
        int line_num;
        char peek;
        unordered_map<string, lextoken_type> str_table;
    public:
        int get_line_num() const;
        Lextoken* scan();
        Lexer();
};

#endif