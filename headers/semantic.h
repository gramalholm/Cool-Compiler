#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "sym_table.h"
#include "parser.h"
#include <vector>

class SemanticAnalyzer{
    public:
        SemanticAnalyzer();
        void analyze(Program* root);
        void collect_classes(Program* root);
        void check_inheritance();
        void check_class(ClassNode* c); //terminar
        void check_method(MethodFeature* mf); //fazer
        void check_attr(AttrFeature* af);// fazer
        string check_expr(Expr* e);
        bool is_subtype(const string& curr, const string&findout);
        string least_comum_ancestor(const string& a, const string& b);
        MethodInfo* lookup_method(const string& classe, const string& method);
        void error(int line, const string& msg);
    private:
        SymbolTable st;
        std::vector<string> errors;
        ClassInfo* curr_class;
        string curr_mtd_return;

};

#endif;