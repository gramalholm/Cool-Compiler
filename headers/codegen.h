#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <utility>

/*
    BrilCodeGen — Gerador de código Bril (formato texto) a partir da AST de Cool.

    Arquitetura: "Pseudo-Visitor" usando dynamic_cast, espelhando o padrão já
    estabelecido no SemanticAnalyzer.  Cada tipo de nó tem um ramo dedicado em
    gen_expr(), o que facilita a extensão futura.

    Estratégia de nomes (SSA-like):
      - Cada expressão produz um novo temporário fresco  vN  (N crescente).
      - Atribuições (AssignExpr) atualizam var_env para apontar ao novo temp,
        preservando a semântica de Cool sem violar o formato Bril.

    Escopo do MVP:
      - Int e Bool  →  traduzidos fielmente.
      - String      →  ignorada (emite const 0 como placeholder).
      - new / attrs →  ignorados (emite const 0 como placeholder).
      - out_int     →  mapeado para a instrução  print  do Bril.
      - self        →  passado como primeiro parâmetro implícito de todo método.
      - Main.main() →  emitido como @main (ponto de entrada Bril, sem params).

    Correções v2:
      - [Problema 1] CallExpr usa Expr::inferred_type do caller para determinar
        o callee correto (ex: @CellularAutomaton_evolve em vez de @Main_evolve).
      - [Problema 2] gen_expr retorna BrilVal = {nome, tipo_bril}, eliminando
        o tipo fixo "int" em AssignExpr e demais instruções de cópia (id).
*/

// Representa o resultado de uma expressão gerada: {nome_variável_bril, tipo_bril}
// Exemplos: {"v3", "int"} | {"v7", "bool"} | {"", ""} para NoExpr/void
using BrilVal = std::pair<std::string, std::string>;

class BrilCodeGen {
public:
    // Ponto de entrada: percorre todo o Program e imprime o .bril em stdout.
    void generate(Program* prog);

private:
    // ── Geradores de nomes únicos ──────────────────────────────────
    int temp_counter  = 0;   // fonte de  v0, v1, v2, ...
    int label_counter = 0;   // fonte de  lbl0, lbl1, lbl2, ...

    std::string fresh_var();    // retorna e incrementa temp_counter
    std::string fresh_label();  // retorna e incrementa label_counter

    // ── Contexto de geração ───────────────────────────────────────
    std::string curr_class_name;

    // Mapeia: nome Cool (variável/param/let) → {nome_bril, tipo_bril}.
    // ATUALIZADO: agora carrega o tipo junto ao nome para resolver o Problema 2.
    std::unordered_map<std::string, BrilVal> var_env;

    // ── Buffer de saída (uma função por vez) ──────────────────────
    // gen_expr emite instruções aqui; gen_method imprime tudo ao final.
    std::ostringstream out;

    // Emite uma instrução indentada com ; no final:  "  instr;\n"
    void emit(const std::string& instr);

    // Emite um label Bril sem indentação:  ".label:\n"
    void emit_label(const std::string& label);

    // Emite uma linha de comentário:  "  # comment\n"
    void emit_comment(const std::string& comment);

    // ── Mapeamento de tipos ───────────────────────────────────────
    // Int  → "int"  |  Bool → "bool"  |  demais → "int" (placeholder)
    std::string cool_to_bril_type(const std::string& cool_type);

    // ── Visitor de expressões ─────────────────────────────────────
    // Emite as instruções Bril correspondentes à expressão 'e'.
    // ATUALIZADO: retorna BrilVal {nome_variável, tipo_bril} em vez de só string.
    // Retorna {"", ""} para expressões sem valor (NoExpr, efeitos puros).
    BrilVal gen_expr(Expr* e);

    // ── Visitors de classe e método ───────────────────────────────
    void gen_class(ClassNode* c);
    void gen_method(ClassNode* c, MethodFeature* mf);
};

#endif // CODEGEN_H
