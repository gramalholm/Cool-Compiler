#ifndef TOKEN_H
#define TOKEN_H
#include <string>

enum class lextoken_type {
    class_token, if_token, then_token, else_token, fi_token, in_kw_token, inherits_token,
    isvoid_token, let_token, loop_token, case_token, eof_token, identifier_token, int_token, string_token, 
    while_token , plus_token, minus_token, esac_token, new_token, of_token, not_token, pool_token,
    error_token, semicolon_token, lparen_token, rparen_token, lbrace_token, rbrace_token, equal_token,
    type_token, true_token, false_token, whitespace_token, comment_token, in_token, out_token,
};

class Lextoken{
    private:
        lextoken_type type;
        std::string token_str;
        int line_num;

    public:

        // Ler os caracteres e no final criar o token com tipo, lexema e linha.
        Lextoken(lextoken_type t, std::string str, int line);
        
        lextoken_type get_type() const;

        const std::string& get_token_str() const;

        int get_line_num() const;
};

#endif