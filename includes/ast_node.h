#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <variant>
#include "production.h"
namespace HParser
{
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
    void adjust_ast(ASTNodePtr &root);
};