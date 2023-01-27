/*
 *   By Htto 2023
 */
#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <list>
#include <set>
#include <sstream>
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
        void push_new_production(Symbol *sym, Production &&prod);
        void update_production(const std::string &name, Symbol *sym, int id, Production new_prod);
        void register_symbol(Symbol *sym, const std::string &name)
        {
            symb_tab.insert({name, sym});
            rsymb_tab.insert({sym, name});
        }
        // if it doesn't exist, then create a new one.
        Symbol *find_sym(const std::string &str, bool is_terminal);
        Symbol *find_sym(HLex::Token tok)
        {
            bool is_ter = tok.tag == terminal;
            std::string str = tok.val.substr(1, tok.val.size() - 2);
            return find_sym(str, is_ter);
        }
        bool nullable(Symbol *sym)
        {
            return null_tab.count(sym);
        }
        std::string get_name(Symbol *sym)
        {
            auto it = rsymb_tab.find(sym);
            if (it == rsymb_tab.end())
                throw std::runtime_error("Context::get_name(Symbol*): unknonwn symbol ");
            return it->second;
        }
        Symbol *get_symbol(const std::string &str)
        {
            auto it = symb_tab.find(str);
            if (it == symb_tab.end())
                throw std::runtime_error("Context::get_symbol(string): unknonwn symbol " + str);
            return it->second;
        }
        void print();

        void print_production(int id)
        {
            std::cout << get_name(prods_left[id]) << "->";
            for (auto item : prods[id].expr)
                std::cout << get_name(item) << " ";
            std::cout << ";";
        }
        
        void calc_nullable();

        void calc_first();

        void calc_follow();
        // calculate first-set ,follow-set nullable-set
        void calc_basic_values();
        // produciton adjustment algorihtm
        void kill_left_commmon_factor();

        void kill_left_recursive();

    public:
    public:
        std::vector<Symbol *> prods_left;
        std::vector<Production> prods;
        std::map<std::string, Symbol *> symb_tab;
        std::map<Symbol *, std::string> rsymb_tab;
        std::set<Symbol *> null_tab;

        // production adjustment
        std::list<int> free_list;
    };

    class Symbol
    {
    public:
        Symbol(Context *_context, bool _is_ter) : context(_context), is_ter(_is_ter) {}
        bool nullable() const;
        bool is_terminal() const { return is_ter; }
        void push_production(int id) { prods.push_back(id); }
        bool left_recursive() const
        {
            for (auto id : prods)
            {
                auto &prod = context->prods[id];
                if (prod.expr.size() && prod.expr.front() == this)
                    return true;
            }
            return false;
        }
        std::set<Symbol *> follows;
        std::set<Symbol *> firsts;
        std::vector<int> prods;

        enum SymType
        {
            COMMON, // common symbol
            LR,     // the generated symbol when kill left recursive node
            LCF,    // the generated symbol when kill left common factors
        } type = COMMON;

    private:
        Context *context;
        bool is_ter;
    };

#ifdef TEST
    const std::string test_file_directory = "./test/production/";
    struct Test
    {
        /*
            INPUT:RULE
            OUTPUT:
            First Line, n nterminal symbols number
            Following Lines, the i-th number format <number> <n-terminial> n <terminals>
        */
        static bool test_first(const std::string &test_file_id)
        {
            std::string input = HLex::read_file(test_file_directory + test_file_id + ".in");
            std::string output = HLex::read_file(test_file_directory + test_file_id + ".out");
            std::map<std::string, std::vector<std::string>> user_res;
            std::map<std::string, std::vector<std::string>> answer_res;
            auto scanner = HLex::scanner(input);
            HParser::Context con(scanner);
            con.calc_nullable();
            con.calc_first();

            for (auto [name, symbol] : con.symb_tab)
            {
                if (symbol->is_terminal())
                    continue;
                for (auto item : symbol->firsts)
                {
                    user_res[name].push_back(con.rsymb_tab[item]);
                }
            }

            std::stringstream ss(output);
            int n;
            ss >> n;
            for (int i = 1; i <= n; i++)
            {
                int num;
                ss >> num;
                std::string symbol;
                ss >> symbol;
                for (int j = 1; j <= num; j++)
                {
                    std::string tmp;
                    ss >> tmp;
                    answer_res[symbol].push_back(tmp);
                }
            }
            return user_res == answer_res;
        }
    };
#endif
}