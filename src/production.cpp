#include "../includes/production.h"
#include <list>
#include <queue>

namespace HParser
{
    using std::pair;
    using std::queue;
    using std::vector;
    
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

        Production calc_common_factors(Context *context, const vector<int> &prod_ids)
        {
            auto first_symbol = context->prods[prod_ids.front()].expr.front();
            Production new_prod;
            new_prod.expr.push_back(first_symbol);
            int ans = 1;
            for (int i = 1;; i++, ans++)
            {
                Symbol *expect_sym = nullptr;
                for (auto prod_id : prod_ids)
                {
                    if (context->prods[prod_id].expr.size() <= i)
                        return new_prod;
                    auto cur = context->prods[prod_id].expr[i];
                    if (expect_sym == nullptr)
                        expect_sym = cur;
                    else if (expect_sym != cur)
                        return new_prod;
                }
                new_prod.expr.push_back(expect_sym);
            }
            return new_prod;
        }
    }
    
    void Context::push_new_production(Symbol *sym, Production &&prod)
    {
        int id = prods.size();
        sym->prods.push_back(id);
        prods_left.push_back(sym), prods.push_back(std::move(prod));
    }
    
    void Context::update_production(const std::string &name, Symbol *sym, int id, Production new_prod)
    {
        sym->prods.push_back(id);
        prods_left[id] = sym;
        prods[id] = new_prod;
        rsymb_tab[sym] = name;
        symb_tab[name] = sym;
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
            std::cout << rsymb_tab[prods_left[i]] << " : ";
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

    void Context::kill_left_commmon_factor()
    {
        auto sym_tab_q = queue<pair<std::string, Symbol *>>();
        for (auto item : symb_tab)
            if (!item.second->is_terminal())
                sym_tab_q.push(item);

        while (!sym_tab_q.empty())
        {
            auto [name, sym] = sym_tab_q.front();
            sym_tab_q.pop();

            std::map<Symbol *, std::vector<int>> common_factor_sets;
            int idx = 0; // gened node idx.
            for (auto i : sym->prods)
            {
                if (prods[i].expr.size() == 0)
                    common_factor_sets[nullptr].push_back(i);
                else
                    common_factor_sets[prods[i].expr.front()].push_back(i);
            }
            // no common factors
            if (common_factor_sets.size() == sym->prods.size())
                continue;
            // do kill left common factors
            // find longest common left factors first
            sym->prods.clear();
            for (auto [common_symbol, prod_ids] : common_factor_sets)
            {
                int ans = 1;
                bool need_to_break = false;
                // left common factor plus a generated Symbol to process different right part
                Production new_prod = calc_common_factors(this, prod_ids);
                const int length = new_prod.expr.size();

                Symbol *new_symbol = new Symbol(this, false);
                std::string new_name = "~" + name + std::to_string(idx++);
                new_symbol->type = Symbol::LCF;

                for (auto id : prod_ids)
                {
                    auto prod = prods[id];
                    prod.expr.erase(prod.expr.begin(), prod.expr.begin() + length);
                    update_production(new_name, new_symbol, id, prod);
                }
                new_prod.expr.push_back(new_symbol);
                push_new_production(sym, std::move(new_prod));
                sym_tab_q.push({new_name, new_symbol});
            }
        }
    }

    void Context::kill_left_recursive()
    {
        for (auto [name, sym] : symb_tab)
        {
            auto &prod_ids = sym->prods;
            // check if the symbol has left recursive productions.
            bool have_left_recursive = sym->left_recursive();
            if (!have_left_recursive)
                continue;
            // A-> A\alpha | \beta
            std::vector<int> alpha_set, beta_set;
            for (auto i : prod_ids)
            {
                if (prods[i].expr.size() && prods[i].expr.front() == sym)
                    alpha_set.push_back(i);
                else
                    beta_set.push_back(i);
            }
            Symbol *new_sym = new Symbol(this, false);
            new_sym->type = Symbol::LR;

            // do kill direct left recursive convert.
            sym->prods.clear();
            for (auto beta : beta_set)
            {
                auto &vcur_prod = prods[beta].expr;
                vcur_prod.push_back(new_sym);
                sym->prods.push_back(beta);
            }
            for (auto alpha : alpha_set)
            {
                auto &vcur_prod = prods[alpha].expr;
                vcur_prod.erase(vcur_prod.begin());
                vcur_prod.push_back(new_sym);
                prods_left[alpha] = new_sym;
                new_sym->prods.push_back(alpha);
            }
            register_symbol(new_sym, name + "'");
            // push epsilon
            push_new_production(new_sym, Production());
        }
    }
}