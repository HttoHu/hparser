#include "../includes/ast_node.h"

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
}