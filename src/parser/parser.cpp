#include "../../headers/parser.h"
#include "../../headers/lexer.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

using namespace std;

void SLRParser::load_grammar() {
    start_symbol = "program";

    gramatica = {
        {"program", {"class_list"}},

        {"class_list", {"class_list", "class", ";"}},
        {"class_list", {"class", ";"}},

        {"class", {"CLASS", "TYPE", "inherits_opt", "{", "feature_list", "}"}},

        {"inherits_opt", {"INHERITS", "TYPE"}},
        {"inherits_opt", {}},

        {"feature_list", {"feature_list", "feature", ";"}},
        {"feature_list", {"feature", ";"}},
 
        {"feature", {"ID", "(", "formal_list", ")", ":", "TYPE", "{", "expr", "}"}},
        {"feature", {"ID", ":", "TYPE", "assign_opt"}},

        {"assign_opt", {"<-", "expr"}},
        {"assign_opt", {}},

        {"formal_list", {"formal_seq"}},
        {"formal_list", {}},

        {"formal_seq", {"formal"}},
        {"formal_seq", {"formal_seq", ",", "formal"}},

        {"formal", {"ID", ":", "TYPE"}},

        {"expr", {"assign"}},

        {"assign", {"ID", "<-", "assign"}},
        {"assign", {"logic"}},

        {"logic", {"logic", "<", "add"}},
        {"logic", {"logic", "<=", "add"}},
        {"logic", {"logic", "=", "add"}},
        {"logic", {"add"}},

        {"add", {"add", "+", "mul"}},
        {"add", {"add", "-", "mul"}},
        {"add", {"mul"}},

        {"mul", {"mul", "*", "unary"}},
        {"mul", {"mul", "/", "unary"}},
        {"mul", {"unary"}},

        {"unary", {"ISVOID", "unary"}},
        {"unary", {"NOT", "unary"}},
        {"unary", {"~", "unary"}},
        {"unary", {"dispatch"}},

        {"dispatch", {"primary"}},
        {"dispatch", {"dispatch", ".", "ID", "(", "args", ")"}},
        {"dispatch", {"dispatch", "@", "TYPE", ".", "ID", "(", "args", ")"}},

        {"primary", {"ID"}},
        {"primary", {"integer"}},
        {"primary", {"string"}},
        {"primary", {"true"}},
        {"primary", {"false"}},
        {"primary", {"(", "expr", ")"}},

        {"primary", {"IF", "expr", "THEN", "expr", "ELSE", "expr", "FI"}},
        {"primary", {"WHILE", "expr", "LOOP", "expr", "POOL"}},

        {"primary", {"{", "block", "}"}},

        {"primary", {"LET", "let_list", "IN", "expr"}},
        {"primary", {"CASE", "expr", "OF", "case_list", "ESAC"}},

        {"primary", {"NEW", "TYPE"}},

        {"block", {"expr_seq"}},

        {"expr_seq", {"expr", ";"}},
        {"expr_seq", {"expr_seq", "expr", ";"}},

        {"let_list", {"let_item"}},
        {"let_list", {"let_list", ",", "let_item"}},

        {"let_item", {"ID", ":", "TYPE", "assign_opt"}},

        {"case_list", {"case_list", "case_item"}},
        {"case_list", {"case_item"}},

        {"case_item", {"ID", ":", "TYPE", "=>", "expr", ";"}},

        {"args", {"arg_list"}},
        {"args", {}},

        {"arg_list", {"expr"}},
        {"arg_list", {"arg_list", ",", "expr"}},

        {"primary", {"ID", "(", "args", ")"}}
    };

    for(auto& prod: gramatica){
        nonterminals.insert(prod.lhs);
    }

    for(auto& prod: gramatica){
        for(auto& sym: prod.rhs){
            if(!sym.empty() && nonterminals.count(sym) == 0){
                terminals.insert(sym);
            }
        }
    }
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
    for(auto& nt: nonterminals){
        first_sets[nt] = {};
    }

    bool changed = true;

    while(changed){
        changed = false;

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
                if(is_terminal(X)){
                    if(!first_sets[lhs].count(X)){
                        first_sets[lhs].insert(X);
                        changed = true;
                    }
                    all_epsilon = false;
                    break;
                }

                for(auto& f: first_sets[X]){
                    if(f != "ε" && !first_sets[lhs].count(f)){
                        first_sets[lhs].insert(f);
                        changed = true;
                    }
                }

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

            if(rhs.empty()) continue;

            for(int i = 0; i < (int)rhs.size(); i++){
                string X = rhs[i];

                if(!is_nonterminal(X)) continue;

                bool all_epsilon = true;

                for(int j = i+1; j < (int)rhs.size(); j++){
                    string B = rhs[j];

                    if(is_terminal(B)){
                        if(!follow_sets[X].count(B)){
                            follow_sets[X].insert(B);
                            changed = true;
                        }
                        all_epsilon = false;
                        break;
                    }

                    for(string s: first_sets[B]){
                        if(s != "ε" && !follow_sets[X].count(s)){
                            follow_sets[X].insert(s);
                            changed = true;
                        }
                    }

                    if(!first_sets[B].count("ε")){
                        all_epsilon = false;
                        break;
                    }
                }

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

        for(const Item& i: result.itens){
            if(i.dot < (int)i.rhs.size()){
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

    for(const Item& i: S.itens){
        if(i.dot < (int)i.rhs.size()){
            if(i.rhs[i.dot] == X){
                Item nw_item{ i.lhs, i.rhs, i.dot + 1};
                result.itens.insert(nw_item);
            }
        }
    }

    return closure(result);
}

int SLRParser::find_state_index(const State& S){
    for(int i = 0; i < (int)states.size(); i++){
        if(states[i].itens == S.itens) return i;
    }
    return -1;
}

int SLRParser::find_production_index(const Item& I){
    for(int i = 0; i < (int)gramatica.size(); i++){
        if(gramatica[i].lhs == I.lhs && gramatica[i].rhs == I.rhs) return i;
    }
    return -1;
}

void SLRParser::build_canonical_collection(){
    Item start_item = {gramatica[0].lhs, gramatica[0].rhs, 0};
    State T;
    T.itens.insert(start_item);
    T = closure(T);
    states.push_back(T);

    for(int i = 0; i < (int)states.size(); i++){
        set<string> symbols;
        for(const Item& I: states[i].itens){
            if(I.dot < (int)I.rhs.size()){
                symbols.insert(I.rhs[I.dot]);
            }
        }

        for(const string& s: symbols){
            State J = gotoState(states[i], s);
            if(J.itens.empty()) continue;

            int index = find_state_index(J);
            if(index == -1){
                states.push_back(J);
                index = static_cast<int>(states.size()) - 1;
            }

            states[i].transitions[s] = index;
        }
    }

    cout << "[lr0] total states: " << states.size() << endl;
}

void SLRParser::build_parsing_table(){
    for(int i = 0; i < (int)states.size(); i++){
        for(auto& [symbol, state] : states[i].transitions){

            if(is_nonterminal(symbol)){
                goto_table[make_pair(i, symbol)] = state;
            }

            if(is_terminal(symbol)){
                string shift = "s" + to_string(state);
                auto key = make_pair(i, symbol);

                action_table[key] = shift;
            }
        }

        for(const Item& I: states[i].itens){
            if(I.dot == (int)I.rhs.size()){
                if(I.lhs == start_symbol && I.rhs == gramatica[0].rhs){
                    action_table[make_pair(i, string("$"))] = "acc";
                    continue;
                }

                int index = find_production_index(I);
                string reduce = "r" + to_string(index);
                auto iterator = follow_sets.find(I.lhs);

                if(iterator != follow_sets.end()){
                    for(const string& s : iterator->second){
                        auto key = make_pair(i, s);
        
                        if (action_table[key].empty() || action_table[key][0] != 's') {
                            action_table[key] = reduce;
                        }
                    }
                }
            }
        }
    }

    cout << "[action_table] size: " << action_table.size() << endl;
    cout << "[goto_table] size: " << goto_table.size() << endl;
}

void SLRParser::parsing(){
    state_stack.push(0);
    Lexer lexer(filepath);
    auto lookahead = lexer.scan();

    set<string> no_ast = {
        "CLASS", "INHERITS", "OF", "ESAC", "IN",
        "THEN", "ELSE", "FI", "LOOP", "POOL",
        "NEW", "ISVOID", "NOT", "~",
        "LET", "CASE", "IF", "WHILE",
        "(", ")", "{", "}", ";", ":", ",", ".",
        "@", "<-", "=>", "+", "-", "*", "/",
        "<", "<=", "="
    };

    while(true){
        int state = state_stack.top();
        string token = lexer.type_to_string(lookahead->get_type());
        auto it = action_table.find(make_pair(state, token));

        if (it == action_table.end()) {
            cout << "Erro sintatico na linha: " << lexer.get_line_num()
                 << " estado: " << state
                 << " token: " <<  token << endl;
            break;
        }

        string act = it->second;

        if(act[0] == 's'){
            int n = stoi(act.substr(1));
            string lexeme;

            if(auto strTok = dynamic_cast<StrToken*>(lookahead.get())){
                lexeme = strTok->get_token_str();
            } else if(auto numTok = dynamic_cast<NumToken*>(lookahead.get())){
                lexeme = std::to_string(numTok->get_token_num());
            } else {
                lexeme = token;
            }

            if(token == "string"){
                ast_stack.push(new StringExpr(lexeme));
            } else if(token == "ID"){
                ast_stack.push(new IdExpr(lexeme));
            } else if(token == "TYPE"){
                ast_stack.push(new IdExpr(lexeme));
            } else if(token == "integer"){
                ast_stack.push(new IntExpr(stoi(lexeme)));
            } else if(token == "true" || token == "false"){
                ast_stack.push(new BoolExpr(lexeme == "true"));
            }

            symbol_stack.push(StackSymbol{token, lexeme});
            state_stack.push(n);
            lookahead = lexer.scan();

        } else if(act[0] == 'r'){
            int n = stoi(act.substr(1));

            if(n < 0 || n >= (int)gramatica.size()){
                cout << "Erro interno no parser: producao invalida " << n << endl;
                break;
            }

            vector<ASTNode*> children;

            auto lhs = gramatica[n].lhs;
            auto rhs = gramatica[n].rhs;

            for(int i = (int)rhs.size() - 1; i >= 0; i--){
                bool collect_child = is_nonterminal(rhs[i]) || no_ast.count(rhs[i]) == 0;

                if(collect_child){
                    if(!ast_stack.empty()){
                        children.push_back(ast_stack.top());
                        ast_stack.pop();
                    } else {
                        children.push_back(nullptr);
                    }
                }

                if(!state_stack.empty()){
                    state_stack.pop();  
                } 
                if(!symbol_stack.empty()){
                    symbol_stack.pop(); 
                }
            }

            reverse(children.begin(), children.end());

            ASTNode* node = build_ast_node(n, children);
            ast_stack.push(node);

            int q_state = state_stack.top();
            symbol_stack.push(StackSymbol{lhs, lhs});

            auto it2 = goto_table.find(make_pair(q_state, lhs));
            if (it2 == goto_table.end()) {
                cout << "Erro sintatico na linha: " << lexer.get_line_num() << endl;
                break;
            }

            state_stack.push(it2->second);

        } else if(act == "acc"){
            cout << "Analise Sintatica feita com sucesso" << endl;
            if(!ast_stack.empty()){
                root = ast_stack.top();
            }
            break;
        } else {
            cout << "Erro sintatico na linha: " << lexer.get_line_num() << endl;
            break;
        }
    }
}

ASTNode* SLRParser::build_ast_node(int production_index, vector<ASTNode*>& children){
    auto& prod = gramatica[production_index];
    const string& lhs = prod.lhs;
    const vector<string>& rhs = prod.rhs;

    // cout << "[ast] r" << production_index << " " << lhs << " children=" << children.size() << endl;

    if(rhs.size() == 0){
        if (lhs == "args") return new ArgsNode({});
        if (lhs == "formal_list") return new FormalListNode({});
        if (lhs == "assign_opt") return nullptr;
        if (lhs == "inherits_opt") return nullptr;
        return nullptr;
    }

    switch(production_index){

        case 0: 
            return children[0];

        case 1: { 
            auto* lista = static_cast<ClassListNode*>(children[0]);
            return new Program(lista->classes);
        }

        case 2: { 
            auto* list = static_cast<ClassListNode*>(children[0]);
            list->classes.push_back(static_cast<ClassNode*>(children[1]));
            return list;
        }

        case 3: 
            return new ClassListNode({static_cast<ClassNode*>(children[0])});

        case 4: { 
            string name = static_cast<IdExpr*>(children[0])->name;
            string parent = "Object";

            if(children[1] != nullptr)
                parent = static_cast<IdExpr*>(children[1])->name;

            auto* fl = static_cast<FeatureListNode*>(children[2]);
            return new ClassNode(name, parent, fl->features);
        }

        case 5: 
            return children[0];

        case 7: { 
            auto* list = static_cast<FeatureListNode*>(children[0]);
            list->features.push_back(static_cast<Feature*>(children[1]));
            return list;
        }

        case 8: 
            return new FeatureListNode({static_cast<Feature*>(children[0])});

        case 9: { 
            string name = static_cast<IdExpr*>(children[0])->name;
            auto* formals = static_cast<FormalListNode*>(children[1]);
            string type = static_cast<IdExpr*>(children[2])->name;
            auto* body = static_cast<Expr*>(children[3]);
            return new MethodFeature(name, formals->formals, type, body);
        }

        case 10: { 
            string name = static_cast<IdExpr*>(children[0])->name;
            string type = static_cast<IdExpr*>(children[1])->name;
            Expr* init = children[2] ? static_cast<Expr*>(children[2]) : nullptr;
            return new AttrFeature(name, type, init);
        }

        case 11: return children[0];
        case 13: return children[0];

        case 15:
            return new FormalListNode({static_cast<Formal*>(children[0])});

        case 16: {
            auto* list = static_cast<FormalListNode*>(children[0]);
            list->formals.push_back(static_cast<Formal*>(children[1]));
            return list;
        }

        case 17: {
            return new Formal(
                static_cast<IdExpr*>(children[0])->name,
                static_cast<IdExpr*>(children[1])->name
            );
        }

        case 18: return children[0];

        case 19:
            return new AssignExpr(
                static_cast<IdExpr*>(children[0])->name,
                static_cast<Expr*>(children[1])
            );

        case 20: return children[0];

        case 21:
            return new BinExpr(LT,
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1])
            );

        case 22:
            return new BinExpr(LE,
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1])
            );

        case 23:
            return new BinExpr(EQ,
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1])
            );

        case 24: return children[0];

        case 25:
            return new BinExpr(PLUS,
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1])
            );

        case 26:
            return new BinExpr(MINUS,
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1])
            );

        case 27:
            return children[0];

        case 28:
            return new BinExpr(MUL,
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1])
            );

        case 29:
            return new BinExpr(DIV,
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1])
            );

        case 30: return children[0];

        case 31:
            return new IsVoidExpr(static_cast<Expr*>(children[0]));

        case 32:
            return new NotExpr(static_cast<Expr*>(children[0]));

        case 33:
            return new NegExpr(static_cast<Expr*>(children[0]));

        case 34: return children[0];

        case 35:
            return children[0];

        case 36:
            return new CallExpr(
                static_cast<Expr*>(children[0]),
                static_cast<IdExpr*>(children[1])->name,
                static_cast<ArgsNode*>(children[2])->args
            );

        case 37:
            return new CallExpr(
                static_cast<Expr*>(children[0]),
                static_cast<IdExpr*>(children[1])->name,
                static_cast<IdExpr*>(children[2])->name,
                static_cast<ArgsNode*>(children[3])->args
            );

        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
            return children[0];

        case 43: 
            return children[0];

        case 44:
            return new IfExpr(
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1]),
                static_cast<Expr*>(children[2])
            );

        case 45:
            return new WhileExpr(
                static_cast<Expr*>(children[0]),
                static_cast<Expr*>(children[1])
            );

        case 46: return children[0];

        case 47:
            return new LetExpr(
                static_cast<LetListNode*>(children[0])->bindings,
                static_cast<Expr*>(children[1])
            );

        case 48:
            return new CaseExpr(
                static_cast<Expr*>(children[0]),
                static_cast<CaseListNode*>(children[1])->branches
            );

        case 49:
            return new NewExpr(static_cast<IdExpr*>(children[0])->name);

        case 50: return children[0];

        case 51:
            return new BlockExpr({static_cast<Expr*>(children[0])});

        case 52: {
            auto* block = static_cast<BlockExpr*>(children[0]);
            block->exprs.push_back(static_cast<Expr*>(children[1]));
            return block;
        }

        case 53:
            return children[0];

        case 54: {
            auto* list = static_cast<LetListNode*>(children[0]);
            auto* item = static_cast<LetListNode*>(children[1]);
            list->bindings.push_back(item->bindings[0]);
            return list;
        }

        case 55: {
            LetBinding b;
            b.name = static_cast<IdExpr*>(children[0])->name;
            b.type = static_cast<IdExpr*>(children[1])->name;
            b.init = children[2] ? static_cast<Expr*>(children[2]) : nullptr;

            auto* node = new LetListNode();
            node->bindings.push_back(b);
            return node;
        }

        case 56: {
            auto* list = static_cast<CaseListNode*>(children[0]);
            auto* item = static_cast<CaseListNode*>(children[1]);
            list->branches.push_back(item->branches[0]);
            return list;
        }

        case 57: return children[0];

        case 58: {
            CaseBranch b;
            b.name = static_cast<IdExpr*>(children[0])->name;
            b.type = static_cast<IdExpr*>(children[1])->name;
            b.expr = static_cast<Expr*>(children[2]);

            auto* node = new CaseListNode();
            node->branches.push_back(b);
            return node;
        }

        case 59: return children[0];

        case 60:
            return new ArgsNode({});

        case 61:
            return new ArgsNode({static_cast<Expr*>(children[0])});

        case 62: {
            auto* list = static_cast<ArgsNode*>(children[0]);
            list->args.push_back(static_cast<Expr*>(children[1]));
            return list;
        }

        case 63:
            return new CallExpr(
                static_cast<IdExpr*>(children[0])->name,
                static_cast<ArgsNode*>(children[1])->args
            );

        default:
            return nullptr;
    }
}

SLRParser::SLRParser(const string& fp) : filepath(fp){
    load_grammar();
    augment_grammar();
    compute_first_sets();
    compute_follow_sets();
    build_canonical_collection();
    build_parsing_table();
}