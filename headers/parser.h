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
using std::pair; 
 
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
    
    bool operator<(const Item& other) const {
        if (lhs != other.lhs) return lhs < other.lhs;
        if (rhs != other.rhs) return rhs < other.rhs;
        return dot < other.dot;
    }
 
    bool operator==(const Item& other) const {
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
 
// ========================= Nó básico para gerar a arvore e poder codar os nós específicos por herança ========================
class ASTNode{
    public: 
        int line;
        ASTNode(int l = 0) : line(l) {}
        virtual ~ASTNode() = default;
};
 
// ======================== program ========================
class Program : public ASTNode{
    public:
        vector<class ClassNode*> classes;
        Program(const vector<ClassNode*>& c) : classes(c) {}
};
 
// ======================== feature em si ========================
class Feature : public ASTNode{
    public:
        virtual ~Feature() = default;
};
// ======================== expr em si ========================
class Expr : public ASTNode{
    public:
        virtual ~Expr() = default;
};
 
// ======================== classe ========================
class ClassNode : public ASTNode{
    public:
        string name;
        string parent;
        vector<Feature*> features;
        ClassNode(const string& n, const string& p, const vector<Feature*>& f) : name(n), parent(p), features(f) {}
 
};
 
class ClassListNode : public ASTNode {
    public:
        vector<ClassNode*> classes;
        ClassListNode(const vector<ClassNode*>& c) : classes(c) {}
};
 
// ======================== Derivação das features ========================
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
 
class FeatureListNode : public ASTNode {
    public:
        vector<Feature*> features;
        FeatureListNode(const vector<Feature*>& f) : features(f) {}
};
 
// ======================== formais (nao terminal adicional pq usando so expr dava conflito) ========================
class Formal : public ASTNode{
    public:
        string name;
        string type;
        Formal(const string& n, const string& t) : name(n), type(t) {}
};
 
class FormalListNode : public ASTNode {
    public:
        vector<Formal*> formals;
        FormalListNode(const vector<Formal*>& f) : formals(f) {}
};
 
// ======================== tipos e identificadores ========================
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
 
// ======================== enum para expressões binarias========================
enum BinOp { PLUS, MINUS, MUL, DIV, LT, LE, EQ };
 
// ======================== expressões binarias ( expr + expr, expr - expr, expr*expr etc) ========================
class BinExpr: public Expr{
    public:
        BinOp op;
        Expr* left;
        Expr* right;
 
        BinExpr(const BinOp& o, Expr* l, Expr* r) : op(o), left(l), right(r) {}
};
 
// ======================== Expressões unárias ( ~id, isvoid, etc) ========================
class IsVoidExpr : public Expr {
    public:
        Expr* expr;
        IsVoidExpr(Expr* e) : expr(e) {}
};
 
class NotExpr : public Expr {
    public:
        Expr* expr;
        NotExpr(Expr* e) : expr(e) {}
};
 
class NegExpr : public Expr {
    public:
        Expr* expr;
        NegExpr(Expr* e) : expr(e) {}
};
 
class NoExpr : public Expr {};
class SelfExpr : public Expr {};
 
// ======================== Assign (um id receber um valor ou expressão etc ( x -> 2 sla )) ========================
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
        BlockExpr(const vector<Expr*>& e) : exprs(e) {}
};
// ======================== Chamada de função ========================
class CallExpr : public Expr {
    public:
        Expr* caller;         
        string method;
        vector<Expr*> args;
 
        bool is_static;
        string static_type;
 
        // chamar a função usando self(exemplo: f())
        CallExpr(const string& m, const vector<Expr*>& a)
            : caller(new SelfExpr()), method(m), args(a),
            is_static(false), static_type("") {}
 
        // chamar a função de uma classe(exemplo: obj.f())
        CallExpr(Expr* obj, const string& m, const vector<Expr*>& a)
            : caller(obj), method(m), args(a),
            is_static(false), static_type("") {}
 
        // chamada da função de uma classe de forma estatica (obj@TYPE.f())
        CallExpr(Expr* obj, const string& st, const string& m, const vector<Expr*>& a)
            : caller(obj), method(m), args(a),
            is_static(true), static_type(st) {}
};
 
class DispatchListNode : public ASTNode {
    public:
        struct Call {
            bool is_static;
            string static_type;
            string method;
            vector<Expr*> args;
        };
 
        vector<Call> calls;
        DispatchListNode() {}
};
// ======================== Let ========================
class LetBinding {
    public:
        string name;
        string type;
        Expr* init; 
};
 
class LetExpr : public Expr{
    public:
        vector<LetBinding> bindings;
        Expr* body;
        LetExpr(const vector<LetBinding>& b, Expr* bd) : bindings(b), body(bd) {}
};
 
class LetListNode : public ASTNode {
public:
    vector<LetBinding> bindings;
};
 
// ======================== Case ========================
class CaseBranch {
    public:
        string name;
        string type;
        Expr* expr;
};
 
class CaseListNode : public ASTNode {
    public:
        vector<CaseBranch> branches;
        CaseListNode() {}
};
 
class CaseExpr : public Expr {
    public:
        Expr* subject;
        vector<CaseBranch> branches;
        CaseExpr(Expr* s, const vector<CaseBranch>& b) : subject(s), branches(b) {}
};
 
// ======================== New ========================
class NewExpr : public Expr{
    public:
        string type;
        NewExpr(const std::string& t) : type(t) {}
};
// ======================== args (nao terminal auxiliar) ========================
class ArgsNode : public ASTNode {
    public:
        vector<Expr*> args;
        ArgsNode(const vector<Expr*>& a) : args(a) {}
};
 
class SLRParser {
    public:
        void parsing();
        SLRParser(const string& filepath = "teste.txt");//feito
        ASTNode* get_root() { return root; }
    private:
        string filepath;
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
