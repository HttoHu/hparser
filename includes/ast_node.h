#pragma once
#include <string>
#include <vector>
#include <memory>
#include <variant>
namespace HParser
{
    struct ASTNode;
    using ASTNodePtr = std::unique_ptr<ASTNode>;
    struct ASTNode
    {
    public:
        using data_type = std::variant<std::monostate, std::string, std::vector<ASTNodePtr>>;

        ASTNode(const std::string &_t, data_type &&_data, bool _is_leaf = false) : type(_t), data(std::move(_data)), is_leaf(_is_leaf) {}
        void print()
        {
            if (is_leaf)
                std::cout << std::get<std::string>(data);
            else
            {
                std::cout << "(" << type<<" ";
                for (auto &item : std::get<std::vector<ASTNodePtr>>(data))
                {
                    item->print();
                    std::cout<<" ";
                }
                std::cout << ")";
            }
        }

    public:
        std::string type;

        bool is_leaf;

        data_type data;
    };
};