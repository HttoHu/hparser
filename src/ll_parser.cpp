#include "../includes/ll_parser.h"
#include "../includes/utils.h"
#include <algorithm>
#include <functional>

namespace HParser
{
    using std::string;
    using std::to_string;
    // code gen utility functions.
    namespace
    {
        template <typename T>
        std::string conv_to_liter(T t)
        {
            return std::to_string(t);
        }
        template <>
        std::string conv_to_liter(std::string str)
        {
            std::map<char, std::string> escape = {
                {'\n', "\\n"}, {'\t', "\\t"}, {'\r', "\\r"}, {'\\', "\\\\"}, {'\'', "\\'"}};

            std::string ret = "\"";
            for (int i = 0; i < str.size(); i++)
            {
                if (escape.count(str[i]))
                    ret += escape[str[i]];
                else
                    ret += str[i];
            }
            ret += '\"';
            return ret;
        }
        template <>
        std::string conv_to_liter(char ch)
        {
            if (ch == 0)
                return "0";
            std::map<char, std::string> escape = {
                {'\n', "\\n"}, {'\t', "\\t"}, {'\r', "\\r"}, {'\\', "\\\\"}, {'\'', "\\'"}};
            if (escape.count(ch))
                return '\'' + escape[ch] + '\'';
            return '\'' + std::string(1, ch) + '\'';
        }
        template <typename K, typename V>
        std::string gen_tab(const std::map<K, V> &mp)
        {
            if (mp.empty())
                return "{}";
            std::string ret = "{";
            int cnt = 0;
            for (auto [k, v] : mp)
            {
                ret += "{" + conv_to_liter(k) + "," + conv_to_liter(v) + "},";
            }
            ret.back() = '}';
            return ret;
        }
        template <typename K, typename V>
        std::string gen_tab2(const std::map<K, V> &mp, std::function<std::string(V)> f)
        {
            if (mp.empty())
                return "{}";
            std::string ret = "{";
            for (auto [k, v] : mp)
            {
                ret += "{" + conv_to_liter(k) + "," + f(v) + "},";
            }
            ret.back() = '}';
            return ret;
        }
        template <typename K, typename V>
        std::string gen_tab3(const std::map<K, V> &mp, std::function<std::string(K, V)> f)
        {
            if (mp.empty())
                return "{}";
            std::string ret = "{";
            for (auto [k, v] : mp)
                ret += f(k, v) + ",";
            ret.back() = '}';
            return ret;
        }
        template <typename V>
        std::string gen_set(const std::set<V> &s)
        {
            if (s.empty())
                return "{}";
            std::string ret = "{";
            for (auto v : s)
                ret += conv_to_liter(v) + ",";
            ret.back() = '}';
            return ret;
        }
        template <typename V>
        std::string gen_set(const std::set<V> &s, std::function<std::string(V)> f)
        {
            if (s.empty())
                return "{}";
            std::string ret = "{";
            for (auto v : s)
                ret += f(v) + ",";
            ret.back() = '}';
            return ret;
        }
        template <typename V>
        std::string gen_vec(const std::vector<V> &s, std::function<std::string(V)> f)
        {
            if (s.empty())
                return "{}";

            std::string ret = "{";
            for (auto v : s)
                ret += f(v) + ",";
            ret.back() = '}';
            return ret;
        }

        std::string gen_func(const std::string &func)
        {
            std::string ret = "[](const std::string &s,int &pos)" + func;
            return ret;
        }

        std::string _gen_symbol_pointer_code(Symbol *sym, const std::string &name, const std::string &v_name, std::map<Symbol *, int> &tab)
        {
            std::string ret;
            ret += name + "->type =(Symbol::SymType)" + std::to_string(sym->type) + ";\n";
            ret += name + "->is_ter = " + to_string(sym->is_terminal()) + ";\n";
            return ret;
        }
        std::string _gen_symbol_vec(const std::vector<Symbol *> &sym_vec, std::map<Symbol *, int> &tab)
        {
            string res = "std::vector<Symbol *> v(" + to_string(sym_vec.size()) + ");\n";
            res += R"(
        for(int i=0;i<v.size();i++){
            v[i] = new Symbol();
        }    
        )";
            for (int i = 0; i < sym_vec.size(); i++)
            {
                res += _gen_symbol_pointer_code(sym_vec[i], "v[" + to_string(i) + "]", "v", tab);
            }
            return res;
        }
        std::string _gen_context(Context *context, std::map<Symbol *, int> &tab)
        {
            string res;
            auto sym_convertor = [&](Symbol *s)
            { return "v[" + to_string(tab[s]) + "]"; };
            for (int i = 0; i < context->prods.size(); i++)
            {
                if (context->prods[i].expr.size())
                {
                    res += "context->prods[" + to_string(i) + "].expr = " + gen_vec<Symbol *>(context->prods[i].expr, sym_convertor) + ";\n";
                }
            }
            return res;
        }
        std::string _gen_parser(const LLParser &parser, std::map<Symbol *, int> &tab)
        {
            string res = "start = " + conv_to_liter(parser.start) + ";\n";
            for (auto &item : parser.ll_tab)
            {
                res += "ll_tab[v[" + to_string(tab[item.first]) + "]]=" + gen_tab3<string, int>(item.second, [](auto k, auto v)
                                                                                                { return "{Tag::" + k + "," + to_string(v) + "}"; }) +
                       ";\n";
            }
            return res;
        }
    }

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

    // generate code.
    std::string LLParser::gen_parser_code(const std::string &temp)
    {
        using namespace Utils;
        // build context first
        Slicer slicer(temp);

        // give every symbol a number
        std::map<Symbol *, int> sym_tab;
        std::vector<Symbol *> sym_vec;
        int cnt = 0;
        for (auto [name, sym] : context->symb_tab)
        {
            sym_tab.insert({sym, cnt++});
            sym_vec.push_back(sym);
        }
        slicer.get_mut("symbol_size") = std::to_string(sym_vec.size());
        slicer.get_mut("type_vec") = gen_vec<Symbol *>(sym_vec, [](const Symbol *sym)
                                                       { return "{" + std::to_string(sym->type) + "," + std::to_string(sym->is_terminal()) + "}"; });
        slicer.get_mut("prods_size") = std::to_string(context->prods.size());
        auto sym_convertor = [&](Symbol *s)
        { return "v[" + to_string(sym_tab[s]) + "]"; };
        slicer.get_mut("prods_left") = gen_vec<Symbol *>(context->prods_left, sym_convertor);
        slicer.get_mut("sym_tab") = gen_tab2<std::string, Symbol *>(context->symb_tab, sym_convertor);

        slicer.get_mut("fill_context_prods") = _gen_context(context.get(), sym_tab);
        slicer.get_mut("fill_ll_tab") = _gen_parser(*this, sym_tab);
        return slicer.merge();
    }
}