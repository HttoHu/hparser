/*
 *   By Htto 2023
 */

#pragma once
#include <vector>
#include <map>
#include <iostream>
#include <string>
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
        // if it doesn't exist, then create a new one.
        Symbol *find_sym(const std::string &str, bool is_terminal);
        Symbol *find_sym(HLex::Token tok)
        {
            bool is_ter = tok.tag == terminal;
            std::string str = tok.val.substr(1, tok.val.size() - 2);
            return find_sym(str, is_ter);
        }
        void print();

        void calc_nullable();

        void calc_first();

        void calc_follow();
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
#define TEST
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