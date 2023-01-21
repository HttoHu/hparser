/*
 *   By Htto 2023
 */

#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <set>
#include "scanner.h"

namespace HParser
{
    const std::string nterminal = "nterminal", terminal = "terminal";

    class Symbol;
    struct Production
    {
        std::vector<Symbol *> expr;
    };

    struct Context
    {
    public:
        Context(HLex::Scanner scanner);
        ~Context();

    public:
        // if it doesn't exist, then create a new one.
        Symbol *find_sym(const std::string &str, bool is_terminal);
        Symbol *find_sym(HLex::Token tok)
        {
            bool is_ter = tok.tag.front() == '<';
            std::string str = tok.val.substr(1, tok.val.size() - 2);
            return find_sym(str, is_ter);
        }
        void print();

        void calc_nullable();

        void calc_first();
        // calculate first-set ,follow-set nullable-set
        void calc_basic_values();

    public:
    public:
        std::vector<Symbol *> prods_left;
        std::vector<Production> prods;
        std::map<std::string, Symbol *> symb_tab;
        std::map<Symbol *, std::string> rsymb_tab;
        std::set<Symbol *> nullable;
    };

    class Symbol
    {
    public:
        Symbol(Context *_context, bool _is_ter) : context(_context), is_ter(_is_ter) {}
        bool nullable() const;
        bool is_terminal() const { return is_ter; }
        void push_production(int id) { prods.push_back(id); }
        const std::vector<int> &get_prods() { return prods; }


        std::set<Symbol *> follows;
        std::set<Symbol *> firsts;

    private:
        Context *context;
        bool is_ter;

        std::vector<int> prods;
    };
}