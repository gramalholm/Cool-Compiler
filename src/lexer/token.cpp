

/*
ta faltando token ainda eu acho (revisar)

tokens devem ser: palavras reservadas, identificadores, literais (string e int), 
simbolos (operadores e pontuacao), EOF, error, comentarios e whitespace (se for necessario para o parser, caso contrario pode ser ignorado)
*/

enum lextoken_type {
    class_token, if_token, then_token, else_token, fi_token, in_kw_token, inherits_token,
    isvoid_token, let_token, loop_token, case_token, eof_token, identifier_token, int_token, string_token, 
    while_token , plus_token, minus_token, esac_token, new_token, of_token, not_token, pool_token,
    error_token, semicolon_token, lparen_token, rparen_token, lbrace_token, rbrace_token, equal_token,
    type_token, true_token, false_token, whitespace_token, comment_token, in_token, out_token,
};

