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
        LLParser(std::unique_ptr<Context> && _context) : context(std::move(_context)){}
        void gen_ll_tab();

        ASTNodePtr parse(const std::vector<HLex::Token> & toks);

        void print_ll_tab();

        std::unique_ptr<Context> context;
        std::map<Symbol*,std::map<std::string,int>> ll_tab;
    };
}
