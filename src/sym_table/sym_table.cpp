#include "../../headers/sym_table.h"

/*
    Adiciona classe a tabela de classes
*/
bool SymbolTable::add_class(const std::string& name, const ClassInfo& c){
    auto it = class_table.find(name);
    if(it != class_table.end())
        return false;

    class_table.insert({name,c});
    return true;
}   

/*
    Dado um nome de uma classe verifica se ela existe na tabela de classes, se existir retorna ela
    se não, retorna nullprt
*/
ClassInfo* SymbolTable::lookup_class(const std::string& name){
    auto it = class_table.find(name);
    if(it != class_table.end())
        return &it->second;
    
    return nullptr;
}

/*
    Entra em um escopo novo dando push.back em um escopo vazio para depois preencher ele
*/
void SymbolTable::enter_scope(){
    scopes.push_back({});
}
/*
    Faz o pop na pilha de escopos para sair do escopo
*/
void SymbolTable::exit_scope(){
    scopes.pop_back();
}

/*
    Adiciona um simbolo nas tabelas de simbolo
*/
bool SymbolTable::add(const std::string& name,const Symbol& symb){
    if(exists_in_current_scope(name))
        return false;
   
    scopes.back().insert({name, symb});
    return true;
};
/*
    Verifica se um simbolo com o nome (name) existe na tabela de simbolos
    se exitir retorna ele
    se não, retorna nullptr
*/
Symbol* SymbolTable::lookup(const std::string& name){
    for(size_t i = scopes.size(); i > 0; i--){
        auto& scope = scopes[i - 1];
        auto it = scope.find(name);
        if(it != scope.end())
            return &it->second;
    }

    return nullptr;
};

/*
    verifica se um simbolo existe no escopo atual
    uso no add para evitar que haja um simbolo duplicado no mesmo escopo
*/
bool SymbolTable::exists_in_current_scope(const std::string& name){
    auto it = scopes.back().find(name);
    if(it != scopes.back().end())
        return true;
    
    return false;
};

/*
    Função auxiliar para usar na analize semantica
*/
std::vector<ClassInfo*> SymbolTable::get_all_classes(){
    std::vector<ClassInfo*> ct;
    for(auto& c: class_table){
        ct.push_back(&c.second);
    }
    return ct;
}