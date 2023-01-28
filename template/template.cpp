/*
 *   The following code is generated by HParser
 *   By Htto 2023
 *   Github: https://github.com/HttoHu/hparser
 */

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <set>
#include <variant>
#include <algorithm>

namespace HLex
{
    struct Token
    {
        std::string tag;
        std::string val;
        std::string to_string() const
        {
            return "<" + tag + "," + val + ">";
        }
    };
}
namespace HParser
{
    namespace
    {
        template <typename T>
        std::vector<T> move_vec(std::vector<T> &v)
        {
            std::vector<T> res;
            for (auto &item : v)
                res.push_back(std::move(item));
            v.clear();
            return res;
        }
    }
    class Symbol
    {
    public:
        bool is_terminal() const { return is_ter; }
        std::set<Symbol *> follows;
        std::set<Symbol *> firsts;
        std::vector<int> prods;

        enum SymType
        {
            COMMON, // common symbol
            LR,     // the generated symbol when kill left recursive node
            LCF,    // the generated symbol when kill left common factors
        } type = COMMON;
        bool is_ter;

    private:
    };

    struct Production
    {
        std::vector<Symbol *> expr;
    };
    struct Context
    {
    public:
        ~Context()
        {
            for (auto [name, sym] : symb_tab)
                delete sym;
        }

    public:
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

    public:
    public:
        std::map<std::string, Symbol *> symb_tab;
        std::map<Symbol *, std::string> rsymb_tab;
        std::vector<Symbol *> prods_left;
        std::vector<Production> prods;
    };
    const int PRINT_INDENT = 2;
    struct ASTNode;
    using ASTNodePtr = std::unique_ptr<ASTNode>;
    struct ASTNode
    {
    public:
        using data_type = std::variant<std::monostate, std::string, std::vector<ASTNodePtr>>;

        ASTNode(const std::string &_t, data_type &&_data, bool _is_leaf = false) : type(_t), data(std::move(_data)), is_leaf(_is_leaf) {}
        void print(int dep = 0)
        {
            for (int i = 0; i < dep; i++)
                std::cout << "|" << std::string(PRINT_INDENT - 1, ' ');
            if (is_leaf)
                std::cout << std::get<std::string>(data) << std::endl;
            else
            {
                std::cout << type << "\n";
                for (auto &item : std::get<std::vector<ASTNodePtr>>(data))
                    item->print(dep + 1);
            }
        }
        // chilren count
        size_t ch_size() const
        {
            return std::get<std::vector<ASTNodePtr>>(data).size();
        }
        ASTNodePtr &operator[](size_t idx)
        {
            return std::get<std::vector<ASTNodePtr>>(data)[idx];
        }
        ASTNodePtr &back()
        {
            return operator[](ch_size() - 1);
        }
        ASTNodePtr &front()
        {
            return operator[](0);
        }
        std::vector<ASTNodePtr> &children()
        {
            std::get<std::vector<ASTNodePtr>>(data);
        }
        Symbol::SymType node_type = Symbol::COMMON;

    public:
        std::string type;
        bool is_leaf;

        data_type data;
    };
    // to recover original parser tree after kill left common factor and left recursive.
    void adjust_ast(ASTNodePtr &node)
    {
        using children_type = std::vector<ASTNodePtr>;
        if (node->is_leaf)
            return;

        children_type new_vec;
        auto &children = std::get<children_type>(node->data);
        if (node->back()->node_type == Symbol::LR)
        {
            auto cur = node->back().get();
            // epsilon
            if (cur->ch_size() == 0)
            {
                if (children.size())
                    children.pop_back();
            }
            else
            {
                // cur_node is an obeserver
                while (cur->ch_size() != 0 && cur->back()->ch_size() != 0)
                    cur = cur->back().get();
                ASTNodePtr new_node = std::make_unique<ASTNode>(node->type, move_vec(children));
                children.push_back(std::move(new_node));

                cur->children().pop_back();
                auto new_right_part = move_vec(cur->children());
                // attach right part
                for (auto &i : new_right_part)
                    children.push_back(std::move(i));
                adjust_ast(node->front());
            }
        }

        for (auto &child : children)
        {
            if (child->node_type == Symbol::COMMON)
            {
                adjust_ast(child);
                new_vec.emplace_back(std::move(child));
            }
            else if (child->node_type == Symbol::LCF)
            {
                adjust_ast(child);
                for (auto &gchild : std::get<children_type>(child->data))
                    new_vec.emplace_back(std::move(gchild));
            }
            else
            {
                std::cout << child->type << "\n";
            }
        }
        children = std::move(new_vec);
    }
    struct LLParser
    {
        LLParser();

        ASTNodePtr parse(const std::vector<HLex::Token> &toks)
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
            adjust_ast(tree_nodes.front());
            return std::move(tree_nodes.front());
        }

        std::string start;
        std::unique_ptr<Context> context;
        std::map<Symbol *, std::map<std::string, int>> ll_tab;
    };

}

int main()
{
    using namespace HParser;
    LLParser parser;
    std::vector<HLex::Token> vecs;
    vecs.push_back({"int", "123"});
    vecs.push_back({"plus", "+"});
    vecs.push_back({"int", "234"});
    vecs.push_back({"mul", "*"});
    vecs.push_back({"int", "345"});
    vecs.push_back({"end", "$"});
    auto res = parser.parse(vecs);
    res->print();
    return 0;
}
