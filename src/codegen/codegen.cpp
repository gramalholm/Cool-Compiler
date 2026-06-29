#include "../../headers/codegen.h"
#include <iostream>

// =============================================================================
// Geradores de nomes únicos
// =============================================================================

std::string BrilCodeGen::fresh_var() {
    return "v" + std::to_string(temp_counter++);
}

std::string BrilCodeGen::fresh_label() {
    return "lbl" + std::to_string(label_counter++);
}

// =============================================================================
// Helpers de emissão
// =============================================================================

void BrilCodeGen::emit(const std::string& instr) {
    out << "  " << instr << ";\n";
}

void BrilCodeGen::emit_label(const std::string& label) {
    out << "." << label << ":\n";
}

void BrilCodeGen::emit_comment(const std::string& comment) {
    out << "  # " << comment << "\n";
}

// =============================================================================
// Mapeamento de tipos Cool → Bril
// =============================================================================

std::string BrilCodeGen::cool_to_bril_type(const std::string& cool_type) {
    if (cool_type == "Int")   return "int";
    if (cool_type == "Bool")  return "bool";
    // String, SELF_TYPE, Object, classes definidas pelo usuário → "int" (referência opaca)
    return "int";
}

// =============================================================================
// Ponto de entrada
// =============================================================================

void BrilCodeGen::generate(Program* prog) {
    for (ClassNode* c : prog->classes) {
        gen_class(c);
    }
}

// =============================================================================
// Visitor de classe
// =============================================================================

void BrilCodeGen::gen_class(ClassNode* c) {
    curr_class_name = c->name;
    for (Feature* f : c->features) {
        if (auto* mf = dynamic_cast<MethodFeature*>(f)) {
            gen_method(c, mf);
        }
        // AttrFeature → ignorado no MVP
    }
}

// =============================================================================
// Visitor de método
// =============================================================================

void BrilCodeGen::gen_method(ClassNode* c, MethodFeature* mf) {
    // ── Resetar estado por função ──────────────────────────────────
    out.str("");
    out.clear();
    temp_counter  = 0;
    label_counter = 0;
    var_env.clear();

    const bool is_entry = (c->name == "Main" && mf->name == "main");
    const std::string func_name = is_entry ? "main" : (c->name + "_" + mf->name);

    // Registrar 'self' e formals no ambiente com seus tipos Bril
    var_env["self"] = {"self", "int"};
    for (Formal* fm : mf->formals) {
        var_env[fm->name] = {fm->name, cool_to_bril_type(fm->type)};
    }

    // Gerar corpo no buffer — BrilVal carrega nome e tipo do resultado
    auto [result_var, result_type] = gen_expr(mf->body);

    // ── Emitir cabeçalho ───────────────────────────────────────────
    if (is_entry) {
        std::cout << "@main {\n";
    } else {
        std::string params = "self: int";
        for (Formal* fm : mf->formals) {
            params += ", " + fm->name + ": " + cool_to_bril_type(fm->type);
        }
        std::string ret_type = cool_to_bril_type(mf->return_type);
        std::cout << "@" << func_name
                  << "(" << params << ")"
                  << ": " << ret_type
                  << " {\n";
    }

    std::cout << out.str();

    // ── Emitir retorno ─────────────────────────────────────────────
    if (is_entry || result_var.empty()) {
        std::cout << "  ret;\n";
    } else {
        std::cout << "  ret " << result_var << ";\n";
    }

    std::cout << "}\n\n";
}

// =============================================================================
// Visitor de expressões — núcleo do gerador
// =============================================================================

/*
    Retorna BrilVal = {nome_variável_bril, tipo_bril}.
    Retorna {"", ""} para expressões sem valor utilizável (NoExpr, void).

    CORREÇÃO Problema 1: CallExpr usa ex->caller->inferred_type (anotado pelo
    SemanticAnalyzer) para construir o nome correto da função chamada.

    CORREÇÃO Problema 2: AssignExpr e todos os usos de 'id' usam o tipo
    dinâmico carregado em BrilVal, eliminando o tipo fixo "int".
*/
BrilVal BrilCodeGen::gen_expr(Expr* e) {
    if (!e) return {"", ""};

    // ── IntExpr ────────────────────────────────────────────────────
    if (auto* ex = dynamic_cast<IntExpr*>(e)) {
        std::string v = fresh_var();
        emit(v + ": int = const " + std::to_string(ex->value));
        return {v, "int"};
    }

    // ── BoolExpr ───────────────────────────────────────────────────
    if (auto* ex = dynamic_cast<BoolExpr*>(e)) {
        std::string v = fresh_var();
        emit(v + ": bool = const " + (ex->value ? "true" : "false"));
        return {v, "bool"};
    }

    // ── StringExpr ─────────────────────────────────────────────────
    if (dynamic_cast<StringExpr*>(e)) {
        std::string v = fresh_var();
        emit_comment("StringExpr ignorada no MVP");
        emit(v + ": int = const 0");
        return {v, "int"};
    }

    // ── NoExpr ─────────────────────────────────────────────────────
    if (dynamic_cast<NoExpr*>(e)) {
        return {"", ""};
    }

    // ── SelfExpr ───────────────────────────────────────────────────
    if (dynamic_cast<SelfExpr*>(e)) {
        return {"self", "int"};
    }

    // ── IdExpr ─────────────────────────────────────────────────────
    // Retorna o BrilVal já armazenado em var_env — inclui o tipo correto.
    if (auto* ex = dynamic_cast<IdExpr*>(e)) {
        auto it = var_env.find(ex->name);
        if (it != var_env.end()) {
            return it->second;  // BrilVal {nome_bril, tipo_bril}
        }
        // Identificador não encontrado (atributo de classe não mapeado no MVP)
        std::string v = fresh_var();
        emit_comment("atributo '" + ex->name + "' nao mapeado no MVP");
        emit(v + ": int = const 0");
        return {v, "int"};
    }

    // ── BinExpr ────────────────────────────────────────────────────
    if (auto* ex = dynamic_cast<BinExpr*>(e)) {
        auto [vl, tl] = gen_expr(ex->left);
        auto [vr, tr] = gen_expr(ex->right);
        std::string vres = fresh_var();

        std::string op, res_type;
        switch (ex->op) {
            case PLUS:  op = "add"; res_type = "int";  break;
            case MINUS: op = "sub"; res_type = "int";  break;
            case MUL:   op = "mul"; res_type = "int";  break;
            case DIV:   op = "div"; res_type = "int";  break;
            case LT:    op = "lt";  res_type = "bool"; break;
            case LE:    op = "le";  res_type = "bool"; break;
            case EQ:    op = "eq";  res_type = "bool"; break;
        }
        emit(vres + ": " + res_type + " = " + op + " " + vl + " " + vr);
        return {vres, res_type};
    }

    // ── NegExpr (negação aritmética ~expr → 0 - expr) ──────────────
    if (auto* ex = dynamic_cast<NegExpr*>(e)) {
        auto [vm, tm]  = gen_expr(ex->expr);
        std::string vzero = fresh_var();
        std::string vres  = fresh_var();
        emit(vzero + ": int = const 0");
        emit(vres  + ": int = sub " + vzero + " " + vm);
        return {vres, "int"};
    }

    // ── NotExpr (negação booleana) ─────────────────────────────────
    if (auto* ex = dynamic_cast<NotExpr*>(e)) {
        auto [vm, tm] = gen_expr(ex->expr);
        std::string vres = fresh_var();
        emit(vres + ": bool = not " + vm);
        return {vres, "bool"};
    }

    // ── IsVoidExpr ─────────────────────────────────────────────────
    if (auto* ex = dynamic_cast<IsVoidExpr*>(e)) {
        gen_expr(ex->expr);  // avaliar por efeitos colaterais
        std::string vres = fresh_var();
        emit_comment("isvoid simplificado para false no MVP");
        emit(vres + ": bool = const false");
        return {vres, "bool"};
    }

    // ── AssignExpr ─────────────────────────────────────────────────
    // Reutiliza o nome Bril já mapeado para 'x' em vez de criar um fresh_var().
    // x <- expr  gera:  x_bril: tipo = id vval;
    // onde x_bril é o mesmo nome que estava em var_env[x].
    // Se x ainda não estiver no env (caso incomum), cria um novo nome.
    if (auto* ex = dynamic_cast<AssignExpr*>(e)) {
        auto [vval, vtype] = gen_expr(ex->expr);   // tipo real da RHS

        // Determinar o nome Bril de destino
        std::string vdest;
        auto it = var_env.find(ex->name);
        if (it != var_env.end()) {
            vdest = it->second.first;  // ← reutiliza o nome já existente
        } else {
            vdest = fresh_var();       // fallback: variável ainda não declarada
        }

        emit(vdest + ": " + vtype + " = id " + vval);
        var_env[ex->name] = {vdest, vtype};   // atualiza tipo (pode ter mudado)
        return {vdest, vtype};
    }

    // ── IfExpr ─────────────────────────────────────────────────────
    //
    //   br vcond .then_N .else_N;
    // .then_N:
    //   <then> → vthen  |  vresult: T = id vthen;  |  jmp .merge_N;
    // .else_N:
    //   <else> → velse  |  vresult: T = id velse;  |  jmp .merge_N;
    // .merge_N:
    //
    // O tipo T vem de inferred_type (anotado pela semântica = LCA dos ramos).
    if (auto* ex = dynamic_cast<IfExpr*>(e)) {
        auto [vcond, tc] = gen_expr(ex->condition);
        std::string l_then  = fresh_label();
        std::string l_else  = fresh_label();
        std::string l_merge = fresh_label();
        std::string vresult = fresh_var();

        // Tipo do resultado: LCA dos dois ramos, vindo da anotação semântica
        std::string res_type = cool_to_bril_type(
            ex->inferred_type.empty() ? "Object" : ex->inferred_type
        );

        emit("br " + vcond + " ." + l_then + " ." + l_else);

        // Ramo then
        emit_label(l_then);
        auto [vthen, tt] = gen_expr(ex->then_branch);
        if (!vthen.empty())
            emit(vresult + ": " + res_type + " = id " + vthen);
        emit("jmp ." + l_merge);

        // Ramo else
        emit_label(l_else);
        auto [velse, te] = gen_expr(ex->else_branch);
        if (!velse.empty())
            emit(vresult + ": " + res_type + " = id " + velse);
        emit("jmp ." + l_merge);

        emit_label(l_merge);
        return {vresult, res_type};
    }

    // ── WhileExpr ──────────────────────────────────────────────────
    //
    //   jmp .cond_N;
    // .cond_N:  br vcond .body_N .end_N;
    // .body_N:  <body>;  jmp .cond_N;
    // .end_N:   vN: int = const 0;   # while retorna Object em Cool
    if (auto* ex = dynamic_cast<WhileExpr*>(e)) {
        std::string l_cond = fresh_label();
        std::string l_body = fresh_label();
        std::string l_end  = fresh_label();

        emit("jmp ." + l_cond);

        emit_label(l_cond);
        auto [vcond, tc] = gen_expr(ex->condition);
        emit("br " + vcond + " ." + l_body + " ." + l_end);

        emit_label(l_body);
        gen_expr(ex->body);   // resultado descartado
        emit("jmp ." + l_cond);

        emit_label(l_end);
        std::string vres = fresh_var();
        emit(vres + ": int = const 0");   // while → Object → int placeholder
        return {vres, "int"};
    }

    // ── BlockExpr ──────────────────────────────────────────────────
    // Avalia todas as sub-expressões em sequência; o valor é o da última.
    if (auto* ex = dynamic_cast<BlockExpr*>(e)) {
        BrilVal last = {"", ""};
        for (Expr* sub : ex->exprs) {
            last = gen_expr(sub);
        }
        return last;
    }

    // ── CallExpr ───────────────────────────────────────────────────
    // CORREÇÃO Problema 1: determina o callee usando inferred_type do caller.
    //
    // Casos especiais IO:
    //   out_int(n)    → print vN;              (instrução nativa Bril)
    //   out_string(s) → ignorado               (Bril não tem string)
    //
    // Caso geral:
    //   SelfExpr como caller  → curr_class_name (não precisa de anotação)
    //   Qualquer outro caller → ex->caller->inferred_type  ← FIX
    if (auto* ex = dynamic_cast<CallExpr*>(e)) {
        auto [vcaller, tc] = gen_expr(ex->caller);

        // ── IO built-in: out_int → instrução print ──
        if (ex->method == "out_int") {
            if (!ex->args.empty()) {
                auto [varg, ta] = gen_expr(ex->args[0]);
                emit("print " + varg);
            }
            return {vcaller, "int"};  // SELF_TYPE → caller
        }

        // ── IO built-in: out_string → ignorado no MVP ──
        if (ex->method == "out_string") {
            for (Expr* arg : ex->args) gen_expr(arg);
            emit_comment("out_string ignorado no MVP");
            return {vcaller, "int"};
        }

        // ── Determinar a classe do callee ─────────────────────────
        // Problema 1 RESOLVIDO:
        //   - Se o caller é 'self': usa curr_class_name diretamente.
        //   - Caso contrário: usa inferred_type do caller, que foi preenchido
        //     pelo SemanticAnalyzer::check_expr (via o wrapper adicionado).
        //   - Fallback para curr_class_name se a anotação não existir.
        std::string caller_class;
        if (dynamic_cast<SelfExpr*>(ex->caller)) {
            caller_class = curr_class_name;
        } else if (!ex->caller->inferred_type.empty()) {
            caller_class = ex->caller->inferred_type;  // ← anotação semântica
        } else {
            caller_class = curr_class_name;  // fallback conservador
        }

        // Montar nome da função Bril
        std::string callee;
        if (ex->is_static) {
            // static_type aqui é o campo de CallExpr (obj@TYPE.method)
            callee = ex->static_type + "_" + ex->method;
        } else {
            callee = caller_class + "_" + ex->method;
        }

        // Tipo de retorno: via anotação semântica do próprio CallExpr
        std::string ret_type = cool_to_bril_type(
            ex->inferred_type.empty() ? "Object" : ex->inferred_type
        );

        // Montar lista de argumentos: self primeiro, depois os demais
        std::string arg_list = vcaller;
        for (Expr* arg : ex->args) {
            auto [varg, ta] = gen_expr(arg);
            arg_list += " " + varg;
        }

        std::string vres = fresh_var();
        emit(vres + ": " + ret_type + " = call @" + callee + " " + arg_list);
        return {vres, ret_type};
    }

    // ── LetExpr ────────────────────────────────────────────────────
    // Cada binding gera uma variável Bril local tipada corretamente.
    // O escopo é restaurado após o corpo (sombreamento).
    if (auto* ex = dynamic_cast<LetExpr*>(e)) {
        // Salvar env atual antes de sombrear com os bindings
        std::unordered_map<std::string, BrilVal> saved_env = var_env;

        for (auto& b : ex->bindings) {
            std::string vbind = fresh_var();
            std::string btype = cool_to_bril_type(b.type);  // tipo declarado

            bool has_init = (b.init != nullptr && !dynamic_cast<NoExpr*>(b.init));
            if (has_init) {
                auto [vinit, vinit_type] = gen_expr(b.init);
                // Usa o tipo real do init (preserva bool se o init for bool)
                emit(vbind + ": " + vinit_type + " = id " + vinit);
                var_env[b.name] = {vbind, vinit_type};
            } else {
                // Inicializador padrão do Cool: 0 para Int/outros, false para Bool
                if (btype == "bool") {
                    emit(vbind + ": bool = const false");
                } else {
                    emit(vbind + ": int = const 0");
                }
                var_env[b.name] = {vbind, btype};
            }
        }

        BrilVal body_result = gen_expr(ex->body);

        // Restaurar escopo: remover/reverter bindings introduzidos por este let
        for (auto& b : ex->bindings) {
            if (saved_env.count(b.name)) {
                var_env[b.name] = saved_env[b.name];
            } else {
                var_env.erase(b.name);
            }
        }

        return body_result;
    }

    // ── CaseExpr ───────────────────────────────────────────────────
    // Despacho por tipo em runtime está além do escopo do MVP.
    if (auto* ex = dynamic_cast<CaseExpr*>(e)) {
        gen_expr(ex->subject);  // avaliar por efeitos colaterais
        std::string vres = fresh_var();
        emit_comment("case nao suportado no MVP");
        emit(vres + ": int = const 0");
        return {vres, "int"};
    }

    // ── NewExpr ────────────────────────────────────────────────────
    // Sem modelo de memória no MVP: const 0 como referência opaca.
    if (auto* ex = dynamic_cast<NewExpr*>(e)) {
        std::string vres = fresh_var();
        emit_comment("new " + ex->type + " ignorado no MVP");
        emit(vres + ": int = const 0");
        return {vres, "int"};
    }

    // ── Fallback ───────────────────────────────────────────────────
    std::string vres = fresh_var();
    emit_comment("expressao nao reconhecida");
    emit(vres + ": int = const 0");
    return {vres, "int"};
}
