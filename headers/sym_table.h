#ifndef SYMTABLE_H
#define SYMTABLE_H

#include <unordered_map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

/*Gabriel:  
    Usando como base o livro "basic compiler design" e os videos do youtube recomendados por bazilio
    Além da tabela de simbolos criei uma tabela de classes também
*/

struct Symbol{
    std::string type;
    std::string kind; // "attr", "param", "let", "case"
    int line;
};

struct AttrInfo{
    std::string name;
    std::string type;
    int line;
}; //atributos da classe

struct MethodInfo{
    std::string name;
    std::string return_type;
    std::vector<std::string> param_types;
    int line;
}; //metodos da classe

struct ClassInfo{
    std::string name;
    std::string parent;
    std::unordered_map<std::string, MethodInfo> methods; // mapa com todos os metodos da classe
    std::unordered_map<std::string, AttrInfo> attrs; // mapa com todos os atributos da classe
};

class SymbolTable {
    public:
        void enter_scope();
        void exit_scope();
        bool add(const std::string& name,const Symbol& symb); //adiciona um simbolo na tabela
        bool add_class(const std::string& name, const ClassInfo& c);
        Symbol* lookup(const std::string& name); // nullptr se não achar
        ClassInfo* lookup_class(const std::string& name);
        bool exists_in_current_scope(const std::string& name); // verifica se tal simbolo etc existe nesse escopo atual
    private:
        std::vector<std::unordered_map<std::string, Symbol>> scopes;
        std::unordered_map<std::string, ClassInfo> class_table;
};

#endif;