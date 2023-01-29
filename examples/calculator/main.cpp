#include <iostream>
#include "parser.h"
namespace Calc
{
    using HParser::ASTNodePtr;
    using std::string;

    using OBJ = double;
    void init();
    OBJ call(const string &node, ASTNodePtr &p);
}
int main()
{
    std::string input;
    std::cin >> input;

    HLex::Lexer L(input);

    auto vec = L.lex();
    // for (auto v : vec)
    // {
    //     std::cout << v.to_string() << "\n";
    // }
    vec.push_back({"end", "$"});
    
    HParser::LLParser parser;
    auto ans = parser.parse(vec);
    
    // ans->print();
    Calc::init();
    std::cout << Calc::call("E", ans->front());
    return 0;
}