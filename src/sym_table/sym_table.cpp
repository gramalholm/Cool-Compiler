#include "../../headers/sym_table.h"

bool SymbolTable::add_class(const std::string& name, const ClassInfo& c){
    auto it = class_table.find(name);
    if(it != class_table.end())
        return false;

    class_table.insert({name,c});
    return false;
}   

ClassInfo* SymbolTable::lookup_class(const std::string& name){
    auto it = class_table.find(name);
    if(it != class_table.end())
        return &it->second;
    
    return nullptr;
}

void SymbolTable::enter_scope(){
    scopes.push_back({});
}

void SymbolTable::exit_scope(){
    scopes.pop_back();
}

bool SymbolTable::add(const std::string& name,const Symbol& symb){
    if(exists_in_current_scope(name))
        return false;
   
    scopes.back().insert({name, symb});
    return true;
};

Symbol* SymbolTable::lookup(const std::string& name){
    for(int i = scopes.size(); i = 0; i--){
        auto it = scopes[i].find(name);
        if(it != scopes[i].end())
            return &it->second;
    }

    return nullptr;
};

bool SymbolTable::exists_in_current_scope(const std::string& name){
    auto it = scopes.back().find(name);
    if(it != scopes.back().end())
        return true;
    
    return false;
};