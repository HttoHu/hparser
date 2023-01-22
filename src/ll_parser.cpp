#include "../includes/ll_parser.h"

namespace HParser
{
    void LLParser::gen_ll_tab()
    {
        context->calc_basic_values();

        for (int i = 0; i < context->prods.size(); i++)
        {
            auto prod = context->prods[i];
            auto left_part = context->prods_left[i];
            std::set<std::string> first_s;
            bool reach_to_end = true;
            for (auto item : prod.expr)
            {
                if (item->is_terminal())
                {
                    first_s.insert(context->rsymb_tab[item]);
                    reach_to_end = false;
                    break;
                }
                else
                {
                    for (auto sym : item->firsts)
                        first_s.insert(context->rsymb_tab[sym]);
                    if (!context->nullable(item))
                    {
                        reach_to_end = false;
                        break;
                    }
                }
            }
            // attach left part follow-set
            if (reach_to_end)
            {
                for (auto sym : left_part->follows)
                    first_s.insert(context->rsymb_tab[sym]);
            }
            // insert the first_s to LL(1) tab
            for (auto tag : first_s)
            {
                if (ll_tab[left_part].count(tag))
                {
                    const std::string rules = "rule " + std::to_string(i) + " and " + std::to_string(ll_tab[left_part][tag]);
                    throw std::runtime_error("LLParser::gen_ll_tab(): LL(1) table conflict, symbol" + context->rsymb_tab[left_part] + " tag " + tag + "have two rules " + rules);
                }
                ll_tab[left_part].insert({tag, i});
            }
        }
    }
    void LLParser::print_ll_tab()
    {
        for (auto [sym, row] : ll_tab)
        {
            std::cout << context->get_name(sym) << ":";
            for (auto [s, i] : row)
                std::cout << "{" << s << "," << i << "}";
            std::cout << "\n";
        }
    }
    ASTNodePtr LLParser::parse(const std::vector<HLex::Token> & toks){

    }
}