#include "lexer.h"
#include <iostream>
#include <fstream>
using std::cin;
using std::cout;

/*
Gabriel:
    muitos comentarios por que estou codando e digitando para eu entender oq to fazendo e ver se faz sentindo e ta certo.
    Alem disso comentarios para que o Léo entenda o que eu fiz e o que eu to fazendo, e para que ele possa me ajudar a corrigir os erros e melhorar o código.
*/

Lexer::Lexer(const string& filepath) {
    file.open(filepath);
    if(!file.is_open()){
        std::cerr << "ERRO: nao foi possivel abrir " << filepath << endl;
    }

    str_table["class"] = lextoken_type::class_token;
    str_table["then"] = lextoken_type::then_token;
    str_table["fi"] = lextoken_type::fi_token;
    str_table["in"] = lextoken_type::in_kw_token;
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

/*
Gabriel:
    Usando ponteiros para retornar os tokens condizendo com o polimorfismo 
    e evitando que a string ou o numero de strtoken ou o numero do num token sejam perdidos.
    OBS: Estou usando o smart-pointer unique_ptr para evitar vazamento de memória 

    Esse método le os caracteres da entrada e faz a analise lexica, ou seja, tira espaços em branco,
    reconhece operadores, palavras reservadas, identificadores, numeros e strings, e retorna o token correspondente.
*/

std::unique_ptr<Lextoken> Lexer::scan() {
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        return std::make_unique<Lextoken>(lextoken_type::eof_token);
    }

    if (!stream_initialized) {
        int first = file.get();

        // correção de erro pq se eu rodasse de novo ia continuar marcando o mesmo erro ou sem erro mesmo se eu fizesse alterações
        if (first == 0xEF) {
            int second = file.get();
            int third = file.get();

            if (second == 0xBB && third == 0xBF) {
                peek = file.get();
            } else {
                if (third != EOF) file.putback(static_cast<char>(third));
                if (second != EOF) file.putback(static_cast<char>(second));
                peek = first;
            }
        } else {
            peek = first;
        }

        stream_initialized = true;
    }

    while(isspace(peek)) {
        if(peek == '\n')
            line_num += 1;
        peek = file.get();
    }

    //cometarios inline
    if(peek == '-'){ 
        peek = file.get();
        if(peek == '-'){
            while(peek != '\n' && peek != EOF){
                peek = file.get();
            }
            return scan(); 
        }
        else{
            return std::make_unique<Lextoken>(lextoken_type::minus_token);
        }
    }

    //comentarios aninhados
    if(peek == '(' && file.peek() == '*'){
        file.get(); 
        int depth = 1;

        while(depth > 0){
            peek = file.get();

            if(peek == EOF){
                return std::make_unique<Lextoken>(lextoken_type::eof_token);
            }

            if(peek == '\n'){
                line_num += 1;
            }

            if(peek == '(' && file.peek() == '*'){
                file.get(); 
                depth++;
            }
            else if(peek == '*' && file.peek() == ')'){
                file.get(); 
                depth--;
            }
        }

        peek = file.get();
        return scan();
    }

    //verifica se são operadores ou pontuação
    switch (peek)
    {
    case '+':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::plus_token);
    case '-':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::minus_token);
    case '*':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::star_token);
    case '/':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::slash_token);
    case '.':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::dot_token);
    case '@':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::at_token);
    case '<':
        peek = file.get();
        if(peek == '-'){
            peek = file.get();
            return std::make_unique<Lextoken>(lextoken_type::assign_token);
        }
        else if(peek == '='){
            peek = file.get();
            return std::make_unique<Lextoken>(lextoken_type::less_equal_token);
        }
        else
            return std::make_unique<Lextoken>(lextoken_type::less_token);
     case '=':
        peek = file.get();
        if(peek == '>'){          // ADICIONADO: reconhece =>
            peek = file.get();
            return std::make_unique<Lextoken>(lextoken_type::arrow_token);
        } else {
            return std::make_unique<Lextoken>(lextoken_type::equal_token);
        }
    case '~':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::tilde_token);
    case ';':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::semicolon_token);
    case ',':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::comma_token);
    case ':':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::colon_token);
    case '(':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::lparen_token);
    case ')':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::rparen_token);
    case '{':
        peek = file.get();
        return std::make_unique<Lextoken>(lextoken_type::lbrace_token);
    case '}':
        peek = file.get();   
        return std::make_unique<Lextoken>(lextoken_type::rbrace_token);
        
    default:
        break;
    }

    // para strings do tipo "frase blablabla"
    if(peek == '"'){
        string aux_str;
        peek = file.get();
        while(peek != '"'){
            if(peek == EOF){
                return std::make_unique<Lextoken>(lextoken_type::eof_token);
            }
            aux_str += peek;
            peek = file.get();
        }
        peek = file.get();
        return std::make_unique<StrToken>(lextoken_type::string_token, aux_str);
    }

    if(isalpha(peek) || peek == '_'){
        string aux_str;

        do{
            aux_str += peek;
            peek = file.get();
        }
        while(isalnum(peek) || peek == '_'); 

        auto str_iterator = str_table.find(aux_str);
    
        if(str_iterator != str_table.end()){
             return std::make_unique<StrToken>(str_iterator->second, aux_str);
        }   
        
        if(std::isupper(static_cast<unsigned char>(aux_str[0]))){
            str_table[aux_str] = lextoken_type::typeID_token; 
            return std::make_unique<StrToken>(lextoken_type::typeID_token, aux_str);
        }else{
            str_table[aux_str] = lextoken_type::objectID_token;
            return std::make_unique<StrToken>(lextoken_type::objectID_token, aux_str);
        }
    }

    if(isdigit(peek)){
        int aux_num = 0;
        do
        {
            aux_num = aux_num * 10 + (peek - '0'); 
            peek = file.get();
        } 
        while (isdigit(peek));

        return std::make_unique<NumToken>(lextoken_type::int_token, aux_num);
        
    }

    if(peek == EOF){
        return std::make_unique<Lextoken>(lextoken_type::eof_token); 
    }

    peek = file.get();
    return std::make_unique<Lextoken>(lextoken_type::error_token);
}

int Lexer::get_line_num() const {
    return line_num;
}

Lextoken::Lextoken(lextoken_type t): type(t) {}

lextoken_type Lextoken::get_type() const {
    return type;
}

StrToken::StrToken(lextoken_type t, std::string str): Lextoken(t), token_str(str) {}

const std::string& StrToken::get_token_str() const {
    return token_str;
}

NumToken::NumToken(lextoken_type t, int num): Lextoken(t), token_num(num) {}

int NumToken::get_token_num() const {
    return token_num;
}

//so para poder printar bonitinho os operadores e pontuações.
string Lexer::type_to_string(lextoken_type t) {
  switch (t)
   {
         // operadores aritméticos
        case lextoken_type::plus_token: return "+";
        case lextoken_type::minus_token: return "-";
        case lextoken_type::star_token: return "*";
        case lextoken_type::slash_token: return "/";
        case lextoken_type::tilde_token: return "~";

        // operadores de comparação
        case lextoken_type::less_token: return "<";
        case lextoken_type::less_equal_token: return "<=";
        case lextoken_type::equal_token: return "=";
        case lextoken_type::arrow_token: return "=>";

        // operadores especiais
        case lextoken_type::assign_token: return "<-";
        case lextoken_type::dot_token: return ".";
        case lextoken_type::at_token: return "@";

        // pontuação
        case lextoken_type::lparen_token: return "(";
        case lextoken_type::rparen_token: return ")";
        case lextoken_type::lbrace_token: return "{";
        case lextoken_type::rbrace_token: return "}";
        case lextoken_type::semicolon_token: return ";";
        case lextoken_type::colon_token: return ":";
        case lextoken_type::comma_token: return ",";

        // palavras reservadas
        case lextoken_type::class_token: return "CLASS";
        case lextoken_type::inherits_token: return "INHERITS";
        case lextoken_type::if_token: return "IF";
        case lextoken_type::then_token: return "THEN";
        case lextoken_type::else_token: return "ELSE";
        case lextoken_type::fi_token: return "FI";
        case lextoken_type::while_token: return "WHILE";
        case lextoken_type::loop_token: return "LOOP";
        case lextoken_type::pool_token: return "POOL";
        case lextoken_type::let_token: return "LET";
        case lextoken_type::in_kw_token: return "IN";
        case lextoken_type::case_token: return "CASE";
        case lextoken_type::esac_token:  return "ESAC";
        case lextoken_type::of_token: return "OF";
        case lextoken_type::new_token: return "NEW";
        case lextoken_type::isvoid_token: return "ISVOID";
        case lextoken_type::not_token: return "NOT";

        // identificadores e literais
        case lextoken_type::objectID_token: return "ID";
        case lextoken_type::typeID_token: return "TYPE";
        case lextoken_type::int_token: return "integer";
        case lextoken_type::string_token: return "string";
        case lextoken_type::true_token: return "true";
        case lextoken_type::false_token: return "false";

        // fim de entrada e erro
        case lextoken_type::eof_token: return "$";
        case lextoken_type::error_token: return "error";

        default: return "UNKNOWN";
    }
}
