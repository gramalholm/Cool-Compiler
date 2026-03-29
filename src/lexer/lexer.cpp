#include "lexer.h"
#include <iostream>
#include <sstream>
using std::cin;
using std::cout;

/*
Gabriel:
    muitos comentarios por que estou codando e digitando para eu entender oq to fazendo e ver se faz sentindo e ta certo.
    Alem disso comentarios para que o Léo entenda o que eu fiz e o que eu to fazendo, e para que ele possa me ajudar a corrigir os erros e melhorar o código.
*/

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

Lextoken* Lexer::is_reserved(string str){
    auto str_iterator = str_table.find(str);
    
    if(str_iterator != str_table.end()){
        return new StrToken(str_iterator->second, str);
    }   

    // Se o Identificador não for encontrado na tabela de símbolos, adiciona-o como um novo token de identificador.
    str_table[str] = lextoken_type::identifier_token; 
    return new StrToken(lextoken_type::identifier_token, str);
}

/*
Gabriel:
    Usando ponteiros para retornar os tokens condizendo com o polimorfismo 
    e evitando que a string ou o numero de strtoken ou o numero do num token sejam perdidos.
    OBS: Estou usando o smart-pointer unique_ptr para evitar vazamento de memória 

    Esse método le os caracteres da entrada e faz a analise lexica, ou seja, tira espaços em branco,
    reconhece operadores, palavras reservadas, identificadores, numeros e strings, e retorna o token correspondente.
*/

std::unique_ptr<Lextoken> Lexer::scan() {

    // retirar espaços em branco
    while(isspace(peek)) {
        if(peek == '\n')
            line_num += 1;
        peek = cin.get();
    }

    if(peek == '*'){
        peek = cin.get();
        while(peek != '*'){
            if(peek == EOF){
                return std::make_unique<Lextoken>(lextoken_type::eof_token); // Retorna um token EOF se o final do arquivo for alcançado antes de fechar o comentário.
            }
            peek = cin.get();
        }
    }

    //verifica se são operadores ou pontuação, se for, retorna o token correspondente
    switch (peek)
    {
    case '+':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::plus_token);
    case '-':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::minus_token);
    case '*':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::star_token);
    case '/':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::slash_token);
    case '.':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::dot_token);
    case '@':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::at_token);
    case '<':
        peek = cin.get();
        if(peek == '-'){
            peek = cin.get();
            return std::make_unique<Lextoken>(lextoken_type::assign_token);
        }
        else if(peek == '='){
            peek = cin.get();
            return std::make_unique<Lextoken>(lextoken_type::less_equal_token);
        }
        else
            return std::make_unique<Lextoken>(lextoken_type::less_token);
    case '>':
        peek = cin.get();
        if(peek == '='){
            peek = cin.get();
            return std::make_unique<Lextoken>(lextoken_type::greater_equal_token);
        }
        else
            return std::make_unique<Lextoken>(lextoken_type::greater_token);
    case '=':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::equal_token);
    case '~':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::tilde_token);
    case ';':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::semicolon_token);
    case ',':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::comma_token);
    case '(':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::lparen_token);
    case ')':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::rparen_token);
    case '{':
        peek = cin.get();
        return std::make_unique<Lextoken>(lextoken_type::lbrace_token);
    case '}':
        peek = cin.get();   
        return std::make_unique<Lextoken>(lextoken_type::rbrace_token);
        
    default:
        break;
    }

    if(isalpha(peek)){
        string aux_str; 
        //ler caracter, verificar se é uma letra, se for, ler lexema completo, verificar se é palavra reservada ou identificador, criar token correspondente
        do{
            aux_str += peek;
            peek = cin.get();
        }
        while(isalpha(peek));

        // Verificar se o lexema é uma palavra reservada ou um identificador e retornar o token correspondente.
        return std::unique_ptr<Lextoken>(is_reserved(aux_str));
    }

    if(isdigit(peek)){
        int aux_num = 0;
        do
        {
            aux_num = aux_num * 10 + (peek - '0'); // Converte o caractere para um número inteiro.
            peek = cin.get();
        } 
        while (isdigit(peek));

        // Retorna um token numérico com o valor lido.
        return std::make_unique<numToken>(lextoken_type::int_token, aux_num);
        
    }

    if(peek == EOF){
        return std::make_unique<Lextoken>(lextoken_type::eof_token); // Retorna um token EOF se o final do arquivo for alcançado.
    }

    // Se o caractere não for reconhecido, retorna um token de erro.
    peek = cin.get();
    return std::make_unique<Lextoken>(lextoken_type::error_token);
}

int Lexer::get_line_num() const {
    return line_num;
}

Lextoken::Lextoken(lextoken_type t): type(t) {}

StrToken::StrToken(lextoken_type t, std::string str): Lextoken(t), token_str(str) {}

numToken::numToken(lextoken_type t, int num): Lextoken(t), token_num(num) {}