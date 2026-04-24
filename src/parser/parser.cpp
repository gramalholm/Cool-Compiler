#include "../../headers/parser.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

void SLRParser::load_grammar() {
    start_symbol = "program";

    gramatica = {
        {"program", {"class_list"}},

        {"class_list", {"class_list", "class", ";"}},
        {"class_list", {"class", ";"}},
        {"class", {"class", "TYPE", "inherits_opt", "{", "feature_list", "}"}},

        {"inherits_opt", {"inherits", "TYPE"}},
        {"inherits_opt", {}},

        {"feature_list", {"feature_list", "feature", ";"}},
        {"feature_list", {}},
        {"feature", {"ID", "(", "formal_list", ")", ":", "TYPE", "{", "expr", "}"}},
        {"feature", {"ID", ":", "TYPE", "assign_opt"}},

        {"assign_opt", {"<-", "expr"}},
        {"assign_opt", {}},

        {"formal_list", {"formal", "formal_list_tail"}},
        {"formal_list", {}},
        {"formal_list_tail", {",", "formal", "formal_list_tail"}},
        {"formal_list_tail", {}},
        {"formal", {"ID", ":", "TYPE"}},

        {"expr_list", {"expr", "expr_list_tail"}},
        {"expr_list", {}},
        {"expr_list_tail", {",", "expr", "expr_list_tail"}},
        {"expr_list_tail", {}},

        {"expr_block", {"expr", ";", "expr_block"}},
        {"expr_block", {"expr", ";"}},

        {"expr", {"let", "let_binding_list", "in", "expr"}},
        {"let_binding", {"ID", ":", "TYPE", "let_init"}},
        {"let_init", {"<-", "expr"}},
        {"let_init", {}},
        {"let_binding_list", {"let_binding", "let_binding_tail"}},
        {"let_binding_tail", {",", "let_binding", "let_binding_tail"}},
        {"let_binding_tail", {}},

        {"expr", {"case", "expr", "of", "case_list", "esac"}},
        {"case_list", {"case_list", "case_elem"}},
        {"case_list", {"case_elem"}},
        {"case_elem", {"ID", ":", "TYPE", "=>", "expr", ";"}},

        {"expr", {"expr", "+", "term"}},
        {"expr", {"expr", "-", "term"}},
        {"expr", {"term"}},

        {"term", {"term", "*", "factor"}},
        {"term", {"term", "/", "factor"}},
        {"term", {"factor"}},

        {"factor", {"primary"}},

        {"primary", {"ID"}},
        {"primary", {"integer"}},
        {"primary", {"string"}},
        {"primary", {"true"}},
        {"primary", {"false"}},
        {"primary", {"(", "expr", ")"}},
        {"primary", {"ID", "(", "expr_list", ")"}},
        {"primary", {"if", "expr", "then", "expr", "else", "expr", "fi"}},
        {"primary", {"while", "expr", "loop", "expr", "pool"}},
        {"primary", {"{","expr_block","}"}},
        {"primary", {"new","TYPE"}},
        {"primary", {"isvoid","expr"}},
        {"primary", {"not","expr"}},
        {"primary", {"~","expr"}},

        {"expr",{"ID","<-","expr"}},
        {"expr",{"expr","@","TYPE",".","ID","(","expr_list",")"}}
    };

    for(auto& prod: gramatica){
        nonterminals.insert(prod.lhs);
    }

    terminals = {
        "TYPE", "ID", "integer", "string",
        "true", "false", "inherits", "in",
        "then", "else", "fi", "loop",
        "pool", "if", "while", "{",
        "}", "new", "isvoid",
        "not", "~", "+", "-",
        "*", "/", "<", "<=",
        "=", ",", ";",
        "=>"
    };
}

void SLRParser::augment_grammar(){
    Production augmented_prod = {"S'", {start_symbol}};
    gramatica.insert(gramatica.begin(), augmented_prod);
    nonterminals.insert("S'");
    start_symbol = "S'";
}

bool SLRParser::is_terminal(const string& Xbol) {
    return terminals.find(Xbol) != terminals.end();
}

bool SLRParser::is_nonterminal(const string& Xbol){
    return nonterminals.find(Xbol) != nonterminals.end();
}

void SLRParser::compute_first_sets() {
    // Implementação do cálculo dos conjuntos FIRST
    for(auto& nt: nonterminals){
        first_sets[nt] = {};
    }

    bool changed = true; // se houver mudanças, o loop continua

    while(changed){
        changed = false; //se não houver mudanças, o loop para

        for(auto& prod: gramatica){
            string lhs = prod.lhs;
            vector<string> rhs = prod.rhs;
              if(rhs.empty()){
                if(!first_sets[lhs].count("ε")){
                    first_sets[lhs].insert("ε");
                    changed = true;
                }
                continue;
            }

            bool all_epsilon = true; 
            
            for(auto& X: rhs){
                //para terminais, adiciona o próprio símbolo ao FIRST(lhs) e para de analisar os próximos símbolos da produção
                if(is_terminal(X)){
                    first_sets[lhs].insert(X);
                    changed = true;
                    all_epsilon = false;
                    break;
                } 

                //para não terminais, adiciona os símbolos de FIRST(X) ao FIRST(lhs), exceto ε
                for(auto& f: first_sets[X]){
                    if(f != "ε" && !first_sets[lhs].count(f)){
                        first_sets[lhs].insert(f);
                        changed = true;
                    }
                }

                //se FIRST(X) contém ε, continua para o próximo símbolo da produção, caso contrário, para de analisar os próximos símbolos da produção
                if (!first_sets[X].count("ε")) {
                    all_epsilon = false;
                    break;
                }

                //se todos os símbolos da produção são nullable, adiciona ε ao FIRST(lhs)
                if(all_epsilon){
                    if(!first_sets[lhs].count("ε")){
                        first_sets[lhs].insert("ε");
                        changed = true;
                }
            }
                
            }
        }
    }
}

void SLRParser::compute_follow_sets(){
    
    for(auto& nt: nonterminals){
        follow_sets[nt] = {};
    }

    follow_sets[start_symbol].insert("$");

    bool changed = true;

    while(changed){
        changed = false;

        for(auto& prod: gramatica){
            string lhs = prod.lhs;
            vector<string> rhs = prod.rhs;

            if(rhs.empty()){
                continue;
            }
            
            for(int i = 0; i < rhs.size(); i++){
                string X = rhs[i];

                if(!is_nonterminal(X)){
                    continue;
                }

                bool all_epsilon = true;

                for(int j = i+1; j < rhs.size(); j++){
                    string B = rhs[j];

                    /*
                        terminais:
                            verificamos se é terminal
                            se for a gente verifica se esse terminal ja esta no follow set desse nao terminal
                            se nao tiver a gente adiciona e quebra a iteração atual
                    */
                    if(is_terminal(B)){
                        if(!follow_sets[X].count(B)){
                            follow_sets[X].insert(B);
                            changed = true;
                        }
                        all_epsilon = false;
                        break;
                    }

                    /*
                        nao terminais:
                            para cada string no first do beta
                            se a stringo nao for epsilon e ainda não estiver contida no follow do nao terminal atual
                            adicionamos as strings do first set do beta no follow do nao terminal atual
                    */
                    for(string s: first_sets[B]){
                        if(s != "ε" && !follow_sets[X].count(s)){
                            follow_sets[X].insert(s);
                            changed = true;
                        }
                    }

                    //se nao tiver epsilon em beta, tornamos all_epsilon falso
                    if(!first_sets[B].count("ε")){
                        all_epsilon = false;
                        break; 
                    }
                }

                // se no final dessa iteração tivermos beta tiver epsilon, adicionamos também ao follow atual o follow do lhs.
                if(all_epsilon){
                    follow_sets[X].insert(follow_sets[lhs].begin(), follow_sets[lhs].end());
                    changed = true;
                }
            }
        }
    }
}

State SLRParser::closure(const State& S){
    State result = S;
    bool changed = true;

    while(changed){
        changed = false;
        vector<Item> new_itens;
        /*Para cada item do estado S (result no caso):
            1)pegamos o simbulo depois do ponto em rhs
            2)buscamos na gramatica as regras de produção com lhs sendo esse simbulo
            3)se nao existir no result, nem em new_items (vetor de items so pra nao dar ko em result durante a iteração)
            4)adicionamos em new items os items da iteração.
            5)depois disso adicionamos o iterator de new_items em result e retornamos ele
        */
        for(const Item& i: result.itens){
            if(i.dot < i.rhs.size()){
                string next_symbol = i.rhs[i.dot];
                for(auto& prod: gramatica){
                    if(prod.lhs != next_symbol) continue;

                    Item nw_item{prod.lhs, prod.rhs, 0};

                    if(result.itens.find(nw_item) == result.itens.end() && 
                    std::find(new_itens.begin(), new_itens.end(), nw_item) == new_itens.end()){

                        new_itens.push_back(nw_item);
                        changed = true;
                    }
                }
            }
        }

        result.itens.insert(new_itens.begin(), new_itens.end());

    }

    return result;
}

State SLRParser::gotoState(const State& S, string X){
    State result;

    /* Criamos um for para rodar por cada item do estado S
        Se o rhs do item na posição do ponto do item for = X:
        Cria um novo item com a posição do ponto + 1
        Insere em resultado
        Retorna esse resultado com a closure aplicada para gerar logo o estado completo.
        Tratar na função que chama gotostate pq ela pode retornar um estado nulo (apenas nao adicionar ele na tabela).
    */
    for(const Item& i: S.itens){
        if(i.dot < i.rhs.size()){
            if(i.rhs[i.dot] == X){
                Item nw_item{ i.lhs, i.rhs, i.dot + 1};
                result.itens.insert(nw_item);
            }
        }
    }

    return closure(result);
}

int SLRParser::find_state_index(const State& S){
    for(int i = 0; i < states.size(); i++){
        if(states[i].itens == S.itens){
            return i;
        }
    }

    return -1;
}

void SLRParser::build_canonical_collection(){
    Item start_item = {gramatica[0].lhs, gramatica[0].rhs, 0};
    State T;

    T.itens.insert(start_item);
    T = closure(T); 
    states.push_back(T);


    for(int i = 0; i < states.size(); i++){
        set<string> symbols; 
        for(const Item& I: states[i].itens){
            if(I.dot < I.rhs.size()){
                symbols.insert(I.rhs[I.dot]);
            }
        }

        for(const string& s: symbols){
            State J = gotoState(states[i], s);

            if(J.itens.empty()) continue;

            int index = find_state_index(J);

            if(index == -1){
                states.push_back(J);
                index = states.size() - 1;
            }

            states[i].transitions[s] = index;

        }
    }
}


