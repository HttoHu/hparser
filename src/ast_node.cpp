#include "../includes/ast_node.h"

namespace HParser
{
    void adjust_ast(ASTNodePtr &node)
    {
        using children_type = std::vector<ASTNodePtr>;
        if (node->is_leaf)
            return;
        children_type new_vec;
        auto &children = std::get<children_type>(node->data);
        for (auto &child : children)
        {
            adjust_ast(child);

            if (child->node_type == Symbol::LCF)
            {
                for (auto &gchild : std::get<children_type>(child->data))
                    new_vec.emplace_back(std::move(gchild));
            }
            else
                new_vec.emplace_back(std::move(child));
        }
        children = std::move(new_vec);
    }
}