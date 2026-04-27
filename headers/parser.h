#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <map>
#include <stack>
#include <utility>
using std::vector;
using std::set;
using std::string;
using std::unordered_map;
using std::map;
using std::stack;

/*  Sites, artigos etc para entender como implementar e teoria de cada parte:
    PARSING SLR NO GERAL) https://web.stanford.edu/class/archive/cs/cs143/cs143.1156/handouts/parsing.pdf
    FIRST E FOLLOW) https://www.geeksforgeeks.org/compiler-design/why-first-and-follow-in-compiler-design/
    CLOSURE e GOTOSTATE) https://www.geeksforgeeks.org/compiler-design/bottom-up-or-shift-reduce-parsers-set-2/ 
*/

struct Production {
    string lhs;
    vector<string> rhs;
};

struct Item {
    string lhs;
    vector<string> rhs;
    int dot; //posição do ponto( 0 <= dot <= tamanho do rhs)
    
    bool operator<(Item& other){
        if (lhs != other.lhs) return lhs < other.lhs;
        if (rhs != other.rhs) return rhs < other.rhs;
        return dot < other.dot;
    }

    bool operator==(Item& other) const {
        return lhs == other.lhs && rhs == other.rhs && dot == other.dot;
    }
};

struct State{
    set<Item> itens; // itens do estado
    map<string,int> transitions; //transição entre 2 estados (int id do proximo estaod) baseado em não terminais e terminais (string)

    bool operator==(const State& other) const {
        return itens == other.itens;
    }
};

struct StackSymbol{
    string type;
    string lexem;
};

/* Gabriel para Léo:
    Aqui basicamente a gente pensa que a arvore sintatica abstrata nao pode ter um nó que faça todas as
    operações, então pra cada nao terminal e suas variações eu criei uma classe:
        Os não terminais herdam do ASTNode
        As variações dos não terminais herdam de seus respectivos não terminais
    (qualquer duvida é so olhar na gramatica que eu extendi no parser.cpp ou olhar o cool manual)
*/
class ASTNode{
    public: 
        int line;
        virtual ~ASTNode() = default;
};

class Program : public ASTNode{
    public:
        vector<class ClassNode*> classes;
        Program(const vector<ClassNode*>& c) : classes(c) {}
};

//classe
class ClassNode : public ASTNode{
    public:
        string name;
        string parent;
        vector<Feature*> features;
        ClassNode(const string& n, const string& p, const vector<Feature*>& f) : name(n), parent(p), features(f) {}

};

class ClassList : public ASTNode {
    public:
        vector<ClassNode*> classes;
};

//Features
class Feature : public ASTNode{};

class MethodFeature : public Feature {
    public:
        string name;
        vector<class Formal*> formals;
        string return_type;
        Expr* body;

        MethodFeature(const string& n, const vector<Formal*>& f,const string& rt, Expr* b)
        : name(n), formals(f), return_type(rt), body(b) {}
};

class AttrFeature : public Feature {
    public:
        string name;
        string type;
        Expr* init; 
        AttrFeature(const string& n, const string& t, Expr* i) : name(n), type(t), init(i) {}

};

class FeatureList : public ASTNode {
public:
    vector<Feature*> features;
};

//formal
class Formal : public ASTNode{
    public:
        string name;
        string type;
        Formal(const string& n, const string& t) : name(n), type(t) {}
};

class FormalList : public ASTNode {
public:
    vector<Formal*> formals;
};

//expressões
class Expr : public ASTNode{};

class ExprList : public ASTNode {
    public:
        vector<Expr*> exprs;
};

class IntExpr: public Expr{
    public:
        int value;
        IntExpr(int v) : value(v) {};
};

class StringExpr : public Expr{
    public:
        string value;
        StringExpr(string s) : value(s){};
};  

class BoolExpr : public Expr{
    public:
        bool value;
        BoolExpr(bool b) : value(b){};
};

class IdExpr : public Expr{
    public:
        string name;
        IdExpr(const string& n) : name(n) {};
};

class BinExpr: public Expr{
    public:
        string op;
        Expr* left;
        Expr* right;

        BinExpr(const string& o, Expr* l, Expr* r) : op(o), left(l), right(r) {}
};

class UnaryExpr : public Expr{
    public:
        string op;
        Expr* expr;
        UnaryExpr(const string& o, Expr* e) : op(o), expr(e) {}
};

class AssignExpr : public Expr{
    public:
        string name;
        Expr* expr;
        AssignExpr(const string& n, Expr* e) : name(n), expr(e) {}
};

class IfExpr : public Expr{
    public:
        Expr* condition;
        Expr* then_branch;
        Expr* else_branch;
        IfExpr(Expr* c, Expr* t, Expr* e) : condition(c), then_branch(t), else_branch(e) {}
};

class WhileExpr : public Expr{
    public:
        Expr* condition;
        Expr* body;

        WhileExpr(Expr* cond, Expr* b) : condition(cond), body(b) {}
};

class BlockExpr: public Expr{
    public:
        vector<Expr*> exprs;
};

class CallExpr : public Expr{
    public:
        Expr*   caller;      
        string  static_type; 
        string  method;
        vector<Expr*> args;
    
        CallExpr(const string& m, const vector<Expr*>& a)
            : caller(nullptr), static_type(""), method(m), args(a) {}
    
        
        CallExpr(Expr* obj, const string& m, const vector<Expr*>& a)
            : caller(obj), static_type(""), method(m), args(a) {}
    
       
        CallExpr(Expr* obj, const string& st, const string& m, const vector<Expr*>& a)
            : caller(obj), static_type(st), method(m), args(a) {}

};

class LetBinding {
    public:
        string name;
        string type;
        Expr* init; 
};

class LetBindingList : public ASTNode {
public:
    vector<LetBinding> bindings;
};

class LetExpr : public Expr{
    public:
        vector<LetBinding> bindings;
        Expr* body;
        LetExpr(const vector<LetBinding>& b, Expr* bd) : bindings(b), body(bd) {}
};

class CaseBranch {
    public:
        string name;
        string type;
        Expr* expr;
};

class CaseList : public ASTNode {
public:
    vector<CaseBranch> branches;
};

class CaseExpr : public Expr {
    public:
        Expr* subject;
        vector<CaseBranch> branches;
        CaseExpr(Expr* s, const vector<CaseBranch>& b) : subject(s), branches(b) {}
};
 

class NewExpr : public Expr{
    public:
        string type;
        NewExpr(const std::string& t) : type(t) {}
};

class SLRParser {
    public:
        void parser();
        SLRParser();//feito
        ASTNode* get_root() { return root; }
    private:
        vector<Production> gramatica;
        set<string> nonterminals;
        set<string> terminals;
        string start_symbol;
        unordered_map<string, set<string>> first_sets; // guarda todos os first sets de cada simbolo
        unordered_map<string, set<string>> follow_sets; // guarda todos os follow sets de cada simbolo
        vector<State> states;
        map<pair<int, string>, int> goto_table;// dado o index de um estado e um simbolo vamos para um outro estado de index diferente.
        map<pair<int, string>, string> action_table;// dado um index de um estado e um simbolo fazemos alguma ação representada pela string.
        stack<int> state_stack;
        stack<StackSymbol> symbol_stack;
        stack<ASTNode*> ast_stack;
        ASTNode* root = nullptr;

        void load_grammar();//feito
        void augment_grammar();//feito
        void compute_first_sets();//feito
        void compute_follow_sets();//feito
        void build_parsing_table();
        bool is_terminal(const string& symbol);//feito
        bool is_nonterminal(const string& symbol);//feito
        State closure(const State& S);//feito
        State gotoState(const State& S, string X);// feito
        int find_state_index(const State& S); //feito
        void build_canonical_collection(); //feito
        void build_parsing_table(); //feito
        int find_production_index(const Item& I); //feito
        ASTNode* build_ast_node(int production_index, vector<ASTNode*>& children);
};

#endif 