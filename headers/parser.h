#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include <map>
using std::vector;
using std::set;
using std::string;
using std::unordered_map;
using std::map;

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
    
    bool operator<(const Item& other){
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


class SLRParser {
    public:
        void load_grammar();//feito
        void augment_grammar();//feito
        void compute_first_sets();//feito
        void compute_follow_sets();//feito
        void build_parsing_table();
        bool is_terminal(const string& symbol);//feito
        bool is_nonterminal(const string& symbol);//feito
        State closure(const State& S);//feito
        State gotoState(const State& S, string X);// feito
        int find_state_index(const State& S);
        void build_canonical_collection();
        //construção dos estados, construção da tabela de parsing
        // buildcanonicalcollection, statesequal, findstateindex
        // redução
        // shift (parse)
        // abstract syntax tree construction
    private:
        vector<Production> gramatica;
        set<string> nonterminals;
        set<string> terminals;
        string start_symbol;
        unordered_map<string, set<string>> first_sets; // guarda todos os first sets de cada simbolo
        unordered_map<string, set<string>> follow_sets; // guarda todos os follow sets de cada simbolo
        vector<State> states;
};

#endif 