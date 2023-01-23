/*
    a Simple LL(1) Parser
*/
#pragma once
#include <iostream>
#include <memory>
#include "production.h"
#include "ast_node.h"

namespace HParser
{
    // LL(1) Parser
    struct LLParser
    {
        LLParser(std::unique_ptr<Context> &&_context, const std::string &_start) : start(_start), context(std::move(_context)) {}
        void gen_ll_tab();

        ASTNodePtr parse(const std::vector<HLex::Token> &toks);

        void print_ll_tab();
        std::string start;
        std::unique_ptr<Context> context;
        std::map<Symbol *, std::map<std::string, int>> ll_tab;
    };
}
