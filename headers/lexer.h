#ifndef LEXER_H
#define LEXER_H
#include <string>
#include <unordered_map>
#include <memory>
#include <fstream>
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
    // identificadores, numeros e strings
    identifier_token,
    int_token,
    string_token,
    bool_token,
    // operadores e pontuacao
    plus_token,
    minus_token,
    star_token,
    slash_token,
    dot_token,
    at_token,
    assign_token,
    equal_token,
    greater_token,
    less_token,
    greater_equal_token,
    less_equal_token,
    not_equal_token,
    arrow_token,
    semicolon_token,
    comma_token,
    colon_token,
    tilde_token,
    lparen_token,
    rparen_token,
    lbrace_token,
    rbrace_token,
    // ERROR e END OF FILE
    error_token,
    eof_token,
    // tokens para tipos
    typeID_token,
    objectID_token,
};
//operadores e pontuacao: + - ; ( ) { } =
class Lextoken{
    private:
        lextoken_type type;
        
    public:
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
class NumToken : public Lextoken {
    private:
        int token_num;

    public:
        NumToken(lextoken_type t, int num);
        int get_token_num() const;
};

class Lexer {
    private:
        int line_num = 1;
        int peek = ' ';
        bool stream_initialized = false;
        std::ifstream file;
        unordered_map<string, lextoken_type> str_table;

    public:
        string type_to_string(lextoken_type t);
        int get_line_num() const;
        std::unique_ptr<Lextoken> scan();
        Lexer(const string& filepath);
};

#endif