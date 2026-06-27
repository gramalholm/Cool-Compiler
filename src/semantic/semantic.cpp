#include "../../headers/semantic.h"
#include "../../headers/sym_table.h"
#include <iostream>
#include <set>

/*
    Construtor da classe onde eu adicionei as classes originarias de COOL:
    Object, Int, String, Bool e IO
*/
SemanticAnalyzer::SemanticAnalyzer(){

    ClassInfo ci_obj = {"Object", "", {}, {}};
    ci_obj.methods["abort"] = {"abort", "Object", {}, 0};
    ci_obj.methods["type_name"] = {"type_name", "String", {}, 0};
    ci_obj.methods["copy"] = {"copy", "SELF_TYPE", {}, 0};
    st.add_class("Object", ci_obj);

    ClassInfo ci_io = {"IO", "Object", {}, {}};
    ci_io.methods["out_string"] = {"out_string", "SELF_TYPE", {"String"}, 0};
    ci_io.methods["out_int"] = {"out_int", "SELF_TYPE", {"Int"}, 0};
    ci_io.methods["in_string"] = {"in_string", "String", {}, 0};
    ci_io.methods["in_int"] = {"in_int", "Int", {}, 0};
    st.add_class("IO", ci_io);

    ClassInfo ci_int = {"Int", "Object", {}, {}};
    st.add_class("Int", ci_int);

    ClassInfo ci_string = {"String", "Object", {}, {}};
    ci_string.methods["length"] = {"length", "Int", {}, 0};
    ci_string.methods["concat"] = {"concat", "String", {"String"}, 0};
    ci_string.methods["substr"] = {"substr", "String", {"Int", "Int"}, 0};
    st.add_class("String", ci_string);

    ClassInfo ci_bool = {"Bool", "Object", {}, {}};
    st.add_class("Bool", ci_bool);
}

/*
    Método principal onde nós chamamos as funções para popular as tabelas e verificamos as classes, tipos e escopos.
*/
void SemanticAnalyzer::analyze(Program* root){
    collect_classes(root);
    check_inheritance();
    for(auto* c: root->classes){
        check_class(c);
    }

    ClassInfo* ci = st.lookup_class("Main");

    if(ci == nullptr)
        error(root != nullptr ? root->line : 0, "classe Main não encontrada");
    else{
        MethodInfo* mi = lookup_method("Main", "main");
        if(mi == nullptr)
            error(ci->line, "método main não encontrado na classe Main");
    }

    if(errors.empty()){
            std::cout << "Programa compilado com sucesso " << std::endl;
    }else{
        for(int i = 0; i < errors.size(); i++){
            std::cout << errors[i] << std::endl;
        }
    }

}
/*
    Método onde, dado a raiz da arvore, descemos para a estrutura de classes dela
    e populamos a tabela de classes
*/
void SemanticAnalyzer::collect_classes(Program* root){
    for(auto cn: root->classes){
        ClassInfo ci;
        ci.name = cn->name;
        ci.line = cn->line;

        if(cn->parent.empty()){
            ci.parent = "Object";
        }else{
            ci.parent = cn->parent;
        }

        for(auto ft: cn->features){
            if(auto* m = dynamic_cast<MethodFeature*>(ft)){
                MethodInfo mi;
                mi.name = m->name;
                mi.return_type = m->return_type;
                for(auto* formal: m->formals){
                    mi.param_types.push_back(formal->type);
                }
                ci.methods[m->name] = mi;
            }
            else if(auto* att = dynamic_cast<AttrFeature*>(ft)){
                AttrInfo ai;
                ai.name = att->name;
                ai.type = att->type;
                ai.line = att->line;
                ci.attrs[att->name] = ai;
            }
        }

        bool c_added = st.add_class(cn->name, ci);

        if(!c_added)
            error(root->line, "Erro ao adicionar a classe, classe duplicada");
    }
}

/*
    Método para guardar em um vetor de erros todos os erros semanticos do programa para depois printar para o usuario
*/
void SemanticAnalyzer::error(int line, const string& msg){
    errors.push_back("Erro na linha " + std::to_string(line) + ": " + msg);
}
/*
    Verificamos nesse método: 
        Para cada classe:
            verificamos se ela não herda das classes originais int, string, bool e io
            verificamos se ela herda de uma classe existente, olhando para a tabela de classes
            verificamos se não há herança ciclica

*/
void SemanticAnalyzer::check_inheritance(){
    std::vector<ClassInfo*> ci = st.get_all_classes();
    for(auto* c: ci){
        std::set<string> fbd = {"Int", "String", "Bool"};
        if(fbd.count(c->parent)){
            error(c->line,"Classe '" + c->name + "' não pode herdar de '" + c->parent + "'");
            continue;
        }
        if(!c->parent.empty() && st.lookup_class(c->parent) == nullptr){
            error(c->line, "Classe " + 
                c->name + "herda da Classe "
                + c->parent + " que não existe");
            continue;
        }else{
            std::set<string> visited;
            ClassInfo* current = c;
            while(current != nullptr && !current->parent.empty()){
                if(visited.count(current->name)){
                    error(current->line, "Herança cíclica envolvendo a classe " 
                        + current->name );
                    break;
                }

                visited.insert(current->name);
                current = st.lookup_class(current->parent);
            }
        }
        
    }
}

/*
    Nesse método verificamos se a classe do nó passado existe
    se existir:
        criamos um novo escopo com enter_scope
        então adicionamos os atributos self, os atributos das classes que ela herda e seus atributos

    depois disso:  
        verificamos os metodos da classe com check_method
        verificamos os atributos da classe com check_attr
    
    saimos do escopo

*/
void SemanticAnalyzer::check_class(ClassNode* c){
    curr_class = st.lookup_class(c->name);

    if(curr_class == nullptr) 
        return;

    st.enter_scope();
    
    if(!st.add("self", {c->name, "self", c->line})){
        error(c->line, "Identificador 'self' já está em uso neste escopo");
    }

    ClassInfo* prt = st.lookup_class(curr_class->parent);
    while(prt != nullptr){
        for(auto& att : prt->attrs){
            if(!st.add(att.first, {att.second.type, "attr", att.second.line})){
                error(att.second.line, "Atributo '" + att.first + "' já foi declarado em uma classe ancestral ou no escopo atual");
            }
        }
        prt = st.lookup_class(prt->parent);
    }
    
    for(auto& att : curr_class->attrs){
        if(!st.add(att.first, {att.second.type, "attr", att.second.line})){
            error(att.second.line, "Atributo '" + att.first + "' já foi declarado em uma classe ancestral ou no escopo atual");
        }
    }

    for(auto* f : c->features){
        if(auto* ft = dynamic_cast<MethodFeature*>(f)){
            check_method(ft);
        }else if(auto* ft = dynamic_cast<AttrFeature*>(f)){
            check_attr(ft);
        }
    }

    st.exit_scope();
}
/*
    Aqui entramos no escopo do método:
        dentro do escopo adicionamos a tabela de simbolos os formals (cada parametro do método)
        setamos o tipo do retorno como o tipo de retorno de mf
        chamamos check_expr no corpo do método para verificarmos os tipos da expressao (ja que body é uma expr)
        verificamos se o tipo do corpo do método é o mesmo que o retorno atual setado
        saimos do escopo
*/
void SemanticAnalyzer::check_method(MethodFeature* mf){
    st.enter_scope();

    for(auto* formal: mf->formals){
        if(!st.add(formal->name, {formal->type, "param", formal->line})){
            error(formal->line, "Parâmetro '" + formal->name + "' já foi declarado neste método");
        }
    }
    curr_mtd_return = mf->return_type;

    string body_type = check_expr(mf->body);
    string expected_return = curr_mtd_return;
    if(!is_subtype(body_type, expected_return))
        error(mf->body->line, "tipo de retorno '" + body_type + "' incompatível com '" + curr_mtd_return + "'");

    st.exit_scope();
}

/*
    verificamos se o inicializador do atributo é nulo
        se não for, verificamos se qual tipo do inicializador usando check_expr
        depois de acharmos o tipo, verificamos se o tipo do inicializador corresponde com o tipo do atributo
        se não for jogamos um erro
*/
void SemanticAnalyzer::check_attr(AttrFeature* af){
    if(af->init != nullptr){
        string ce = check_expr(af->init);
        if(!is_subtype(ce, af->type)){
            error(af->line, "Tipo do inicializador do atributo '" + af->name + "' não é compatível: esperado '" + af->type + "', recebeu '" + ce + "'");
        }
    }
}

/*
    Basicamente a função mais importante do código.
        Aqui dado a uma expressão passada, nós verificamos o tipo inferindo ele por dynamic_cast
        a depender do tipo, retornamos o tipo da expressão

        aqui, verificamos se os tipos de x+y, ou seja, de operações, atribuições, todo tipo de expressão estão corretos

        é a função mais importante pois aqui fazemos a verificação de tipos do programa
*/
string SemanticAnalyzer::check_expr(Expr* e){
    if(auto* ex = dynamic_cast<IntExpr*>(e)){
        return "Int";
    }else if(auto* ex = dynamic_cast<StringExpr*>(e)){
        return "String";
    }else if(auto* ex = dynamic_cast<BoolExpr*>(e)){
        return "Bool";
    }else if(auto* ex = dynamic_cast<IdExpr*>(e)){
        Symbol* symb = st.lookup(ex->name);
        if(symb == nullptr){
            error(ex->line, "Identificador " + ex->name + " não declarado");
            return "Object";
        }else{
            return symb->type;
        }
    }else if(auto* ex = dynamic_cast<SelfExpr*>(e)){
        return curr_class->name;
    }else if(auto* ex = dynamic_cast<BinExpr*>(e)){
        string left = check_expr(ex->left);
        string right = check_expr(ex->right);

        if(ex->op == PLUS || ex->op == MINUS || ex->op == MUL || ex->op == DIV){
            if(left != "Int" || right != "Int"){
                error(ex->line, "Operadores aritméticos esperam Int mas receberam '" + left + "' e '" + right + "'");
            }
            return "Int";

        }else if(ex->op == LT || ex->op == LE){
            if(left != "Int" || right != "Int"){
                error(ex->line, "Operadores de comparação esperam Int mas receberam '" + left + "' e '" + right + "'");
            }
            return "Bool";

        }else if(ex->op == EQ){
            bool left_prim  = (left  == "Int" || left  == "String" || left  == "Bool");
            bool right_prim = (right == "Int" || right == "String" || right == "Bool");
            if((left_prim || right_prim) && left != right){
                error(ex->line, "Operador = entre tipos primitivos exige mesmo tipo, recebeu '" + left + "' e '" + right + "'");
            }
            return "Bool";
        }

        return "Object";
    }else if(auto* ex = dynamic_cast<IsVoidExpr*>(e)){
        string ce = check_expr(ex->expr);
        return "Bool";

    }else if(auto* ex = dynamic_cast<NotExpr*>(e)){
        string ce = check_expr(ex->expr);
        if(ce != "Bool")
            error(ex->line, "NOT espera Bool mas recebeu " + ce);
        
        return "Bool";

    }else if(auto* ex = dynamic_cast<NegExpr*>(e)){
        string ce = check_expr(ex->expr);
        if(ce != "Int")
            error(ex->line, "NOT espera Int mas recebeu " + ce);
        return "Int";

    }else if(auto* ex = dynamic_cast<AssignExpr*>(e)){
        Symbol* var = st.lookup(ex->name);
        if(var == nullptr){
            error(ex->line, "Variavel " + ex->name + "não foi declarada");
            return "Object";
        }

        string tp = check_expr(ex->expr);

        if(!is_subtype(tp, var->type)){
            error(ex->line, "não é possível atribuir tipo '" + tp + "' à variável '" + ex->name + "' do tipo '" + var->type + "'");
        }

        return var->type;
    }else if(auto* ex = dynamic_cast<IfExpr*>(e)){
        string ce = check_expr(ex->condition);
        if(ce != "Bool"){
            error(ex->line, "condição do if deve ser Bool mas recebeu '" + ce + "'");
        }

        string aux_t = check_expr(ex->then_branch);
        string aux_e = check_expr(ex->else_branch);

       return least_comum_ancestor(aux_t, aux_e);

    }else if(auto* ex = dynamic_cast<WhileExpr*>(e)){
        string ce = check_expr(ex->condition);
        if(ce != "Bool"){
            error(ex->line, "condição do while deve ser Bool mas recebeu '" + ce + "'");
        }
        check_expr(ex->body);
        return "Object";
    }else if(auto* ex = dynamic_cast<BlockExpr*>(e)){
        string ce;
        for(auto* e: ex->exprs){
            ce = check_expr(e);
        }
        return ce;
    }else if(auto* ex = dynamic_cast<CallExpr*>(e)){
        string ce = check_expr(ex->caller);
        if(ex->is_static){
            MethodInfo* mi = lookup_method(ex->static_type, ex->method);
            if(mi == nullptr){
                error(ex->line, "Método chamado não existe na classe " + ce);
                return "Object";
            }
            if(ex->args.size() != mi->param_types.size()){
                error(ex->line, "Numero de parametros não corresponde com o método");
            }
            for(int i = 0; i<ex->args.size(); i++){
                string cex = check_expr(ex->args[i]);
                if(!is_subtype(cex, mi->param_types[i])){
                    error(ex->args[i]->line, "O tipo do argumento " + std::to_string(i + 1) + " não é compatível: esperado '" + mi->param_types[i] + "', recebeu '" + cex + "'");
                }
            }
            if(mi->return_type == "SELF_TYPE"){
                return ce;
            }
            return mi->return_type;
        }else{
            MethodInfo* mi = lookup_method(ce, ex->method);
            if(mi == nullptr){
                error(ex->line, "Método chamado não existe na classe " + ce);
                return "Object";
            }
            if(ex->args.size() != mi->param_types.size()){
                error(ex->line, "Numero de parametros não corresponde com o método");
            }
            for(int i = 0; i < ex->args.size(); i++){
                string cex = check_expr(ex->args[i]);
                if(!is_subtype(cex, mi->param_types[i])){
                    error(ex->args[i]->line, "O tipo do argumento " + std::to_string(i + 1) + " não é compatível: esperado '" + mi->param_types[i] + "', recebeu '" + cex + "'");
                }
            }
            if(mi->return_type == "SELF_TYPE"){
                return ce;
            }
            return mi->return_type;
        }
    }else if(auto* ex = dynamic_cast<NewExpr*>(e)){
        ClassInfo* ci = st.lookup_class(ex->type);
        if(ci == nullptr){
            error(ex->line, "tipo '" + ex->type + "' não foi declarado");
            return "Object";
        }
        return ex->type;
    }else if(auto* ex = dynamic_cast<LetExpr*>(e)){
        st.enter_scope();
        for(auto& b: ex->bindings){
            if(b.init != nullptr){
                string init_type = check_expr(b.init);
                if(!is_subtype(init_type, b.type)){
                    error(b.init->line, "tipo do init '" + init_type + "' incompatível com '" + b.type + "'");
                }
            } else {
            }
            if(!st.add(b.name, {b.type, "let", b.init != nullptr ? b.init->line : 0})){
                error(b.init != nullptr ? b.init->line : 0, "Variável '" + b.name + "' já foi declarada neste let");
            }
        }
        string ce = check_expr(ex->body);
        st.exit_scope();
        return ce;
    }else if(auto* ex = dynamic_cast<CaseExpr*>(e)){
        string ce = check_expr(ex->subject);
        std::set<string> branch_types;
        string lca;
        bool first_branch = true;
        for(auto& b: ex->branches){
            if(branch_types.count(b.type)){
                error(b.expr->line, "Tipo '" + b.type + "' já foi usado em outro branch do case");
            }else{
                branch_types.insert(b.type);
            }
            st.enter_scope();
            st.add(b.name, {b.type, "case", b.expr->line});
            string cex = check_expr(b.expr);
            if(first_branch){
                lca = cex;
                first_branch = false;
            }else{
                lca = least_comum_ancestor(lca, cex);
            }
            st.exit_scope();
        }
        return lca;
    }else{
        return "Object";
    }
}

/*
    Método para verificar se uma classe é subtipo da outra
*/

bool SemanticAnalyzer::is_subtype(const string& curr, const string& findout){
    string target = findout;
    if(target == "SELF_TYPE" && curr_class != nullptr){
        target = curr_class->name;
    }

    string atual = curr;
    while(!atual.empty()){
        if(atual == target) return true;

        ClassInfo* ci = st.lookup_class(atual);

        if(ci == nullptr) return false;
        atual = ci->parent;
    }
    return false;
}
/*
    Método que dado duas classes, verificamos qual a classe que ambas herdam que está mais proxima das duas
*/
string SemanticAnalyzer::least_comum_ancestor(const string& a, const string& b){
    std::set<string> ancestor;
    ClassInfo* current = st.lookup_class(a);

    while(current != nullptr){
        ancestor.insert(current->name);
        current = st.lookup_class(current->parent);
    }

    ClassInfo* curr_b = st.lookup_class(b);
    while(curr_b != nullptr){
        if(ancestor.count(curr_b->name)){
            return curr_b->name;
        }
        curr_b = st.lookup_class(curr_b->parent);
    }

    return "Object";
}
/*
    Método que dado uma classe e um método verificamos se esse método existe na classe dada
*/
MethodInfo* SemanticAnalyzer::lookup_method(const string& classe, const string& method){
    ClassInfo* current = st.lookup_class(classe);
    while(current != nullptr){
        if(current->methods.count(method))
            return &current->methods[method];
        current = st.lookup_class(current->parent);
    }
    return nullptr;
}
