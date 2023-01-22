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
        std::string type;

        bool is_leaf;

        std::variant<std::monostate, std::string, std::vector<ASTNodePtr>> data;
    };
};