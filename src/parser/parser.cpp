#include "../../headers/parser.h"
#include "../../headers/lexer.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;
/* Comentarios:
    tive que abrir a gramatica apra evitar [[]]*, [[]]+ e []
    tive que modificar as expr pois elas eram ambiguas 
    e isso da erro no parser slr(1) pois geraria mais de uma ação em cada par na tabela action
*/

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
                    if(!first_sets[lhs].count(X)){
                        first_sets[lhs].insert(X);
                        changed = true;
                    }
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
            }

            if(all_epsilon){
                if(!first_sets[lhs].count("ε")){
                    first_sets[lhs].insert("ε");
                    changed = true;
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
                    for(const string& s : follow_sets[lhs]){
                        if(!follow_sets[X].count(s)){
                            follow_sets[X].insert(s);
                            changed = true;
                        }
                    }
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

int SLRParser::find_production_index(const Item& I){
    for(int i = 0; i < gramatica.size(); i++){
        if(gramatica[i].lhs == I.lhs && gramatica[i].rhs == I.rhs){
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

    /*Itera para cada estado nos vetores de estados:
        declara um conjunto de simbolos
        para cada item do estado state[i]:
            se o simbolo estiver na posição do ponto adiciona ao conjunto
        depois itera para cada simbolo no conjunto de simbolos:
            fazemos um goto do estado states[i] para o simbolo s atual
            se J nao for vazio verifica se ele ja existe em states
            se nao existir adiciona em states, atualiza o index dele
            depois de atualizar adiciona a transição no map de transições do estado state[i]
    */

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

void SLRParser::build_parsing_table(){
    for(int i = 0; i < states.size(); i++){
        for(auto& [symbol, state] : states[i].transitions){
            if(is_nonterminal(symbol)){
                auto key = make_pair(i, symbol);
                goto_table[key] = state;
            }

            if(is_terminal(symbol)){
                string shift = "s" + to_string(state);
                auto key = make_pair(i, symbol);

                if (!action_table[key].empty()) {
                    cout << "Grammar is not SLR(1): conflict at state "
                        << i << " symbol " << symbol << endl;
                    exit(1);
                }

                action_table[key] = shift;
            }
        }

        for(const Item& I: states[i].itens){
            if(I.dot == I.rhs.size()){
                if(I.lhs == start_symbol && I.rhs == gramatica[0].rhs){
                    auto key = make_pair(i, string("$"));
                    action_table[key] = "acc";
                    continue;
                }

                int index = find_production_index(I);
                string reduce = "r" + to_string(index);
                auto iterator = follow_sets.find(I.lhs);

                if(iterator != follow_sets.end()){
                    for(const string& s : iterator->second){
                        auto key = make_pair(i, s);
                        // if para verificar se ja existe uma ação para aquele par
                        if (!action_table[key].empty()) {
                            cout << "Grammar is not SLR(1): conflict at state "
                                << i << " symbol " << s << endl;
                            exit(1);
                        }
                        action_table[key] = reduce;
                    }
                }
            }
        }
    }
}

  /* Gabriel para Leo:
  entramos com o estado inicial (0) na pilha de estados
        lemos o lookahead
        entramos em um while
        pegamos o elemento no topo da lista de estados e o lookahead e olhamos a pilha de action
        verificamos se o act é vazio para nao dar erro sintatico
        se for shift:   
            empilhamos o estado que o shift diz para ir ex: s6 empilhamos o 6
            empilhamos o token na pilha de simbolos
            lemos o proximo lookahead
        se for reduce:
            pegamos a regra de produção que o reduce pede "rn" pegamos a regra n
            tiramos da pilha de estados e de simbolos em um for do tamanho do rhs
            pegamos o estado no topo da pilha depois de desempilharmos
            achamos o estado na tabela goto, verificamos se não é vazio 
            empilhamos o simbolo de lhs na stack de simbolos
            empilhamos o estado da tabela goto
        se for acc:
            deu tudo certo, break
        se for erro (act vazio):
            erro sintatico, e damos um break também
    
    */
void SLRParser::parser(){
    state_stack.push(0);
    Lexer lexer;
    auto lookahead = lexer.scan();
  
    while(true){
        int state = state_stack.top();
        string token = lexer.type_to_string(lookahead->get_type());
        auto it = action_table.find(make_pair(state, token));

        if (it == action_table.end()) {
            cout << "Erro sintatico na linha: " << lexer.get_line_num() << endl;
            break;
        }

        string act = it->second;

        if (act.empty())
        {
            cout << "Erro sintatico na linha: " << lexer.get_line_num() << endl;
            break;
        }

        if(act[0] == 's')
        {
            int n = stoi(act.substr(1));
            string lexeme;

            if(auto strTok = dynamic_cast<StrToken*>(lookahead.get())){
                lexeme = strTok->get_token_str();
            }
            else if(auto numTok = dynamic_cast<NumToken*>(lookahead.get())){
                lexeme = std::to_string(numTok->get_token_num());
            }
            else{
                lexeme = token; // operadores, palavras-chave
            }

            if(token == "string"){
                ast_stack.push(new StringExpr(lexeme));
            }else if(token == "ID"){
                ast_stack.push(new IdExpr(lexeme));
            }else if(token == "integer"){
                ast_stack.push(new IntExpr(stoi(lexeme)));
            }else if(token == "true" || token == "false"){
                ast_stack.push(new BoolExpr(lexeme == "true"));
            }

            symbol_stack.push(StackSymbol{token, lexeme});
            state_stack.push(n);
            lookahead = lexer.scan();
        //codar a parte de criar a arvore ast quando é reduce e a função build ast node
        }else if(act[0] == 'r'){
            int n = stoi(act.substr(1));
            vector<ASTNode*> children;

            auto lhs = gramatica[n].lhs;
            auto rhs = gramatica[n].rhs;
            
            for(int i = 0; i < rhs.size(); i++){
                symbol_stack.pop();
                state_stack.pop();

                //adicionando nós filhos da arvore no vetor de filhos e tirando eles da stack
                if(!ast_stack.empty()){
                    children.push_back(ast_stack.top());
                    ast_stack.pop();
                }
            }
            //invertendo a ordem do vetor pois ao adicionar na pilha primeiro a gente pegaria o da direita e nao o da esquerda
            reverse(children.begin(), children.end());

            //criando nó pai olhando os filhos e a regra de produção n.
            ASTNode* node = build_ast_node(n, children);

            if(node != nullptr){
                ast_stack.push(node);
            }

            int q_state = state_stack.top();
            symbol_stack.push(StackSymbol{lhs, lhs});

            auto it2 = goto_table.find(make_pair(q_state, lhs));

            if (it2 == goto_table.end()) {
                cout << "Erro sintatico na linha: " << lexer.get_line_num() << endl;
                break;
            }

            state_stack.push(it2->second);

        }else if(act == "acc"){
            cout << "Analise Sintatica feita com sucesso" << endl;

            if(!ast_stack.empty()){
                root = ast_stack.top();
            }

            break;
        }else{
            cout << "Erro sintatico na linha: " << lexer.get_line_num() << endl;
            break;
        }
    }
}

/* Gabriel para Leo:
    Aqui basicamente a gente pega a regra de produção que a gente esta reduzindo
    então a gente pega o lhs dela e o rhs e verifica qual é o lhs
    a depender o lhs a gente gera um nó e retorna ele
    implementei basicamente a logica de cada nó baseado na gramatica extendida que a gente criou ali em cima
    a gente verifica se o rhs é vazio e retorna o nó correspondente as listas das classes ou algumas outras lhs
    pq só elas derivam para episilon
*/

ASTNode* SLRParser::build_ast_node(int production_index, vector<ASTNode*>& children){
    auto& prod = gramatica[production_index];
    const string& lhs = prod.lhs;
    const vector<string>& rhs = prod.rhs;

    if (rhs.empty()) {
        if (lhs == "expr_list"  || lhs == "expr_list_tail")  return new ExprList();
        if (lhs == "formal_list"|| lhs == "formal_list_tail") return new FormalList();
        if (lhs == "feature_list") return new FeatureList();
        if (lhs == "class_list") return new ClassList();
        if (lhs == "case_list") return new CaseList();
        if (lhs == "inherits_opt") return new IdExpr("");
        if (lhs == "assign_opt") return nullptr;
        if (lhs == "let_init") return nullptr;
        if (lhs == "let_binding_tail") return new LetBindingList();
        return nullptr;
    }

    if(prod.rhs.size() == 1 && !children.empty()){
        return children[0];

    } else if(lhs == "program"){
        auto* class_list = static_cast<ClassList*>(children[0]);
        return new Program(class_list->classes);

    } else if(lhs == "class_list" && rhs.size() == 2){  // class ;
        auto* class_list = new ClassList();
        class_list->classes.push_back(static_cast<ClassNode*>(children[0]));
        return class_list;

    } else if(lhs == "class_list" && rhs.size() == 3){
        auto* class_list = static_cast<ClassList*>(children[0]);
        class_list->classes.push_back(static_cast<ClassNode*>(children[1]));
        return class_list;

    } else if(lhs == "class"){
        string name = static_cast<IdExpr*>(children[0])->name;
        string parent = static_cast<IdExpr*>(children[1])->name; // "" se vazio
        auto*  feature_list = static_cast<FeatureList*>(children[2]);
        return new ClassNode(name, parent, feature_list->features);

    } else if(lhs == "inherits_opt"){
        return children[0];

    } else if(lhs == "feature_list" && rhs.size() == 3){
        auto* feature_list = static_cast<FeatureList*>(children[0]);
        feature_list->features.push_back(static_cast<Feature*>(children[1]));
        return feature_list;

    } else if(lhs == "feature" && rhs[1] == "("){
        string name = static_cast<IdExpr*>(children[0])->name;
        auto* feature_list = static_cast<FormalList*>(children[1]);
        string return_type = static_cast<IdExpr*>(children[2])->name;
        Expr* body = static_cast<Expr*>(children[3]);
        return new MethodFeature(name, feature_list->formals, return_type, body);

    } else if(lhs == "feature" && rhs[1] == ":"){
        string name = static_cast<IdExpr*>(children[0])->name;
        string type = static_cast<IdExpr*>(children[1])->name;
   
        if(children.size() > 2){
            Expr* init = static_cast<Expr*>(children[2]);
            return new AttrFeature(name, type, init);
        }else{
            Expr* init = nullptr;
            return new AttrFeature(name, type, init);
        }

    } else if(lhs == "assign_opt"){
        return children[0]; 

    } else if (lhs == "formal") {
        string name = static_cast<IdExpr*>(children[0])->name;
        string type = static_cast<IdExpr*>(children[1])->name;
        return new Formal(name, type);

    } else if(lhs == "formal_list"){
        auto* formal_list = new FormalList();
        formal_list->formals.push_back(static_cast<Formal*>(children[0]));
        auto* tail = static_cast<FormalList*>(children[1]);
       formal_list->formals.insert(formal_list->formals.end(), tail->formals.begin(), tail->formals.end());
        return formal_list;

    } else if(lhs == "formal_list_tail"){
        auto* formal_list= new FormalList();
        formal_list->formals.push_back(static_cast<Formal*>(children[0]));
        auto* tail = static_cast<FormalList*>(children[1]);
        formal_list->formals.insert(formal_list->formals.end(), tail->formals.begin(), tail->formals.end());
        return formal_list;

    } else if(lhs == "expr_list"){
        auto* expr_list = new ExprList();
        expr_list->exprs.push_back(static_cast<Expr*>(children[0]));
        auto* tail = static_cast<ExprList*>(children[1]);
        expr_list->exprs.insert(expr_list->exprs.end(), tail->exprs.begin(), tail->exprs.end());
        return expr_list;

    } else if(lhs == "expr_list_tail"){
        auto* expr_list = new ExprList();
        expr_list->exprs.push_back(static_cast<Expr*>(children[0]));
        auto* tail = static_cast<ExprList*>(children[1]);
        expr_list->exprs.insert(expr_list->exprs.end(), tail->exprs.begin(), tail->exprs.end());
        return expr_list;

    } else if(lhs == "expr_block"){
        if(rhs.size() == 3){
            auto* block = static_cast<BlockExpr*>(children[1]);
            block->exprs.insert(block->exprs.begin(), static_cast<Expr*>(children[0]));
            return block;
        }else{
            auto* block = new BlockExpr();
            block->exprs.push_back(static_cast<Expr*>(children[0]));
            return block;
        }

    } else if(lhs == "expr"){
        if(rhs.size() == 3 && rhs[1] == "<-"){
            string name = static_cast<IdExpr*>(children[0])->name;
            return new AssignExpr(name, static_cast<Expr*>(children[1]));
        }
        if(rhs.size() == 3 && (rhs[1] == "+" || rhs[1] == "-")){
            return new BinExpr(rhs[1],
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1]));
        }
        if(rhs.size() == 8 && rhs[1] == "@"){
            Expr* obj = static_cast<Expr*>(children[0]);
            string s = static_cast<IdExpr*>(children[1])->name; // TYPE
            string method = static_cast<IdExpr*>(children[2])->name; // ID
            auto* args = static_cast<ExprList*>(children[3]);

            return new CallExpr(obj, s, method, args->exprs);
        }
        if(rhs[0] == "let"){
            auto* binding_list = static_cast<LetBindingList*>(children[0]);
            Expr* body = static_cast<Expr*>(children[1]);

            return new LetExpr(binding_list->bindings, body);
        }
        if(rhs[0] == "case"){
            Expr* expr = static_cast<Expr*>(children[0]);
            CaseList* case_list = static_cast<CaseList*>(children[1]);

            return new CaseExpr(expr, case_list->branches);
        }
    } else if(lhs == "term" && rhs.size() == 3){
        return new BinExpr(rhs[1],
            static_cast<Expr*>(children[0]),
            static_cast<Expr*>(children[2]));

    } else if(lhs == "primary"){

        if(rhs[0] == "(") return children[0];

        if(rhs.size() == 4 && rhs[0] == "ID"){
            string s = static_cast<IdExpr*>(children[0])->name;
            auto* args = static_cast<ExprList*>(children[1]);
            return new CallExpr(s, args->exprs);
        }

        if(rhs[0] == "if"){
            return new IfExpr(
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1]),
                static_cast<Expr*>(children[2]));
        }

        if(rhs[0] == "while"){
            return new WhileExpr(
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1]));
        }

        if(rhs[0] == "{") return children[0];

        if(rhs[0] == "new"){
            return new NewExpr(static_cast<IdExpr*>(children[0])->name);
        }

        if(rhs[0] == "isvoid" || rhs[0] == "not" || rhs[0] == "~"){
            return new UnaryExpr(rhs[0], static_cast<Expr*>(children[0]));
        }

    } else if(lhs == "let_binding"){
        LetBinding b;
        b.name = static_cast<IdExpr*>(children[0])->name;
        b.type = static_cast<IdExpr*>(children[1])->name;

        if(children.size() > 2){
            b.init = static_cast<Expr*>(children[2]);
        }else{
            b.init = nullptr;
        }

        auto* binding_list = new LetBindingList();
        binding_list->bindings.push_back(b);
        return binding_list;

    } else if(lhs == "let_init"){
        return children[0];

    } else if(lhs == "let_binding_list"){
        auto* head = static_cast<LetBindingList*>(children[0]);
        auto* tail = static_cast<LetBindingList*>(children[1]);
        head->bindings.insert(head->bindings.end(), tail->bindings.begin(), tail->bindings.end());
        return head;

    } else if(lhs == "let_binding_tail"){
        auto* head = static_cast<LetBindingList*>(children[0]);
        auto* tail = static_cast<LetBindingList*>(children[1]);
        head->bindings.insert(head->bindings.end(), tail->bindings.begin(), tail->bindings.end());
        return head;

    } else if(lhs == "case_elem"){
        CaseBranch branch;
        branch.name = static_cast<IdExpr*>(children[0])->name;
        branch.type = static_cast<IdExpr*>(children[1])->name;
        branch.expr = static_cast<Expr*>(children[2]);
        auto* cl = new CaseList();
        cl->branches.push_back(branch);
        return cl;

    } else if(lhs == "case_list" && rhs.size() == 1){
        return children[0];

    } else if(lhs == "case_list" && rhs.size() == 2) {
        auto* case_list = static_cast<CaseList*>(children[0]);
        auto* elem = static_cast<CaseList*>(children[1]);
        case_list->branches.insert(case_list->branches.end(), elem->branches.begin(), elem->branches.end());
        return case_list;
    }

    if (!children.empty()) return children[0];
    return nullptr;
}

SLRParser::SLRParser(){
    load_grammar();
    augment_grammar();
    compute_first_sets();
    compute_follow_sets();
    build_canonical_collection();
    build_parsing_table();
}