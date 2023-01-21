#include "../includes/production.h"

namespace HParser
{
    Context::Context(HLex::Scanner scanner)
    {
        scanner.print();
        while (!scanner.is_end())
        {
            auto symbol_token = scanner.get_cur_token();
            scanner.match(nterminal);

            auto symbol = find_sym(symbol_token);
            scanner.match("colon");

            while (true)
            {
                Production prod;
                while (scanner.get_cur_token().tag != "semi" && scanner.get_cur_token().tag != "or")
                {
                    prod.expr.push_back(find_sym(scanner.get_cur_token()));
                    if (scanner.cur_tag() != nterminal && scanner.cur_tag() != terminal)
                        scanner.match(nterminal);
                    else
                        scanner.next();
                }
                prods.push_back(prod);
                prods_left.push_back(symbol);
                if (scanner.cur_tag() == "or")
                    scanner.match("or");
                else if(scanner.cur_tag() == "semi")
                    break;
            }
            scanner.match("semi");
            scanner.skip();
        }
    }
    Context::~Context()
    {
        for (auto [s, item] : symb_tab)
            delete item;
    }

    Symbol *Context::find_sym(const std::string &str, bool is_terminal)
    {
        auto it = symb_tab.find(str);
        if (it == symb_tab.end())
        {
            auto ret = new Symbol(this, is_terminal);
            symb_tab.insert({str, ret});
            rsymb_tab.insert({ret, str});
            return ret;
        }
        if (it->second->is_terminal() != is_terminal)
            throw std::runtime_error("find_sym(): terminal nterminal shared same name!");
        return it->second;
    }
    void Context::print()
    {
        for (int i = 0; i < prods.size(); i++)
        {
            std::cout << rsymb_tab[prods_left[i]] << ":";
            for (auto item : prods[i].expr)
            {
                std::cout << rsymb_tab[item] << " ";
            }
            std::cout << "|;"[i == prods.size() - 1] << "\n";
        }
    }
}