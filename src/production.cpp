#include "../includes/production.h"
#include <list>

namespace HParser
{
    namespace
    {
        template <typename T>
        // return if s1 changed.
        bool merge_set(std::set<T> &s1, const std::set<T> &s2)
        {
            int old_sz = s1.size();
            s1.insert(s2.begin(), s2.end());
            return s1.size() != old_sz;
        }
    }
    Context::Context(HLex::Scanner scanner)
    {
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

                symbol->push_production(prods.size());
                prods.push_back(prod);
                prods_left.push_back(symbol);

                if (scanner.cur_tag() == "or")
                    scanner.match("or");
                else if (scanner.cur_tag() == "semi")
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

    void Context::calc_nullable()
    {
        std::list<std::pair<Symbol *, Production *>> L;
        for (int i = 0; i < prods.size(); i++)
            L.push_back({prods_left[i], &prods[i]});
        bool changed = true;
        while (changed)
        {
            changed = false;
            for (auto it = L.begin(); it != L.end();)
            {
                auto production = it->second;
                auto symbol = it->first;
                if (null_tab.count(symbol))
                {
                    it = L.erase(it);
                    continue;
                }
                bool enable_null = true;
                bool is_ter = false;
                for (auto item : production->expr)
                {
                    if (item->is_terminal())
                    {
                        it = L.erase(it);
                        is_ter = true;
                        break;
                    }
                    else if (!null_tab.count(item))
                    {
                        enable_null = false;
                        break;
                    }
                }
                if (is_ter)
                    continue;
                if (enable_null)
                {
                    null_tab.insert(symbol);
                    changed = true;
                }
                it++;
            }
        }
    }
    // fixed point algorithm.

    void Context::calc_first()
    {
        std::list<std::pair<Symbol *, Production *>> L;

        std::map<Symbol *, int> symb_cnt_tab;

        for (int i = 0; i < prods.size(); i++)
        {
            L.push_back({prods_left[i], &prods[i]});
            symb_cnt_tab[prods_left[i]]++;
        }
        bool changed = true;
        while (changed)
        {
            changed = false;
            for (auto it = L.begin(); it != L.end();)
            {
                auto symbol = it->first;
                auto production = it->second;
                if (production->expr.size() == 0)
                {
                    symb_cnt_tab[symbol]--;
                    it = L.erase(it);
                    continue;
                }

                bool enable_to_delete = true;
                for (auto item : production->expr)
                {
                    if (item->is_terminal())
                    {
                        if (!symbol->firsts.count(item))
                        {
                            symbol->firsts.insert(item);
                            changed = true;
                        }
                        break;
                    }
                    else
                    {
                        if (symb_cnt_tab[item])
                            enable_to_delete = false;
                        auto tp = symbol->firsts.size();
                        // First(symbol) |= First(item);
                        symbol->firsts.insert(item->firsts.begin(), item->firsts.end());
                        changed |= symbol->firsts.size() != tp;
                    }
                    if (!null_tab.count(item))
                        break;
                }
                if (enable_to_delete)
                    it = L.erase(it), symb_cnt_tab[symbol]--;
                else
                    it++;
            }
        }
    }

    void Context::calc_follow()
    {
        std::list<std::pair<Symbol *, Production *>> L;

        std::map<Symbol *, int> symb_cnt_tab;

        for (int i = 0; i < prods.size(); i++)
        {
            L.push_back({prods_left[i], &prods[i]});
            symb_cnt_tab[prods_left[i]]++;
        }

        bool changed = true;
        while (changed)
        {
            changed = false;
            // iterate every production
            for (auto it = L.begin(); it != L.end();)
            {
                auto symbol = it->first;
                auto production = it->second;
                bool enable_delete = true;
                std::set<Symbol *> work_set = symbol->follows;

                // iterate production from a[n] to a[1]
                for (int i = production->expr.size() - 1; i >= 0; i--)
                {
                    auto item = production->expr[i];
                    if (item->is_terminal())
                        work_set = {item};
                    else
                    {
                        changed |= merge_set(item->follows, work_set);
                        enable_delete &= (symb_cnt_tab[item] == 0);

                        if (!null_tab.count(item))
                            work_set = item->firsts;
                        else
                            merge_set(work_set, item->firsts);
                    }
                }
                // if (enable_delete)
                //     it = L.erase(it);
                // else
                    it++;
            }
        }
    }

    void Context::calc_basic_values()
    {
        calc_nullable();
        calc_first();
        calc_follow();
    }
}