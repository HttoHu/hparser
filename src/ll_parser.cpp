#include "../includes/ll_parser.h"
#include <algorithm>

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
            {
                std::cout << "{" << s << ",";
                context->print_production(i);
                std::cout << "}";
            }
            std::cout << "\n";
        }
    }
    ASTNodePtr LLParser::parse(const std::vector<HLex::Token> &toks)
    {
        struct StacNode
        {
            enum Type
            {
                SYMBOL,
                POP
            } type;
            StacNode(Type t, Symbol *sym) : type(t), symbol(sym) {}
            Symbol *symbol;
            // if the type is pop, then pop such nodes
            int production_symbol_cnt = 0;
        };

        int pos = 0;
        std::vector<StacNode> parsing_stac;
        std::vector<ASTNodePtr> tree_nodes;

        // init parsing_stac with reverse order. because it is a stack
        auto start_symbol = context->get_symbol(start);
        parsing_stac.push_back(StacNode(StacNode::SYMBOL, start_symbol));

        while (parsing_stac.size())
        {
            auto top_symbol = parsing_stac.back();
            parsing_stac.pop_back();
            if (top_symbol.type == StacNode::SYMBOL)
            {
                if (pos >= toks.size())
                    throw std::runtime_error("LLParser::parse: unexpected end");
                auto cur_tok = toks[pos];
                if (top_symbol.symbol->is_terminal())
                {
                    if (cur_tok.tag == context->get_name(top_symbol.symbol))
                    {
                        auto node = std::make_unique<ASTNode>(cur_tok.tag, std::string(cur_tok.val), true);
                        node->node_type = Symbol::COMMON;
                        tree_nodes.push_back(std::move(node));
                    }
                    else
                        throw std::runtime_error("syntax error! unexpected token " + cur_tok.to_string());
                    pos++;
                }
                else
                {
                    auto it = ll_tab[top_symbol.symbol].find(cur_tok.tag);
                    if (it == ll_tab[top_symbol.symbol].end())
                        throw std::runtime_error("syntax error! unexpected token " + cur_tok.to_string());
                    auto &production = context->prods[it->second].expr;
                    parsing_stac.push_back(StacNode(StacNode::POP, context->prods_left[it->second]));
                    parsing_stac.back().production_symbol_cnt = production.size();
                    for (int i = (int)production.size() - 1; i >= 0; i--)
                        parsing_stac.push_back(StacNode(StacNode::SYMBOL, production[i]));
                }
            }
            else if (top_symbol.type == StacNode::POP)
            {
                std::string node_type = context->get_name(top_symbol.symbol);
                int cnt = top_symbol.production_symbol_cnt;
                if (tree_nodes.size() < cnt)
                    throw std::runtime_error("ASTNodePtr LLParser::parse :internal error!");
                std::vector<ASTNodePtr> nodes;
                // get back nodes and merge to a new node
                while (cnt)
                    nodes.push_back(std::move(tree_nodes.back())), tree_nodes.pop_back(), cnt--;
                std::reverse(nodes.begin(), nodes.end());
                auto new_node = std::make_unique<ASTNode>(node_type, std::move(nodes));
                new_node->node_type = top_symbol.symbol->type;
                tree_nodes.push_back(std::move(new_node));
            }
        }
        if (tree_nodes.size() != 1)
            throw std::runtime_error("LLParser::parse : syntax error!");
        return std::move(tree_nodes.front());
    }

}