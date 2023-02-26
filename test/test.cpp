#include "parser.h"

int main()
{
    HLex::Lexer lex("1+1");
    auto token_stream = lex.lex();
    token_stream.push_back({HLex::END,"END"});
    for (auto item : token_stream)
        std::cout << item.val << "\n";
    HParser::LLParser parser;
    auto node = parser.parse(token_stream);
    node->print();
    return 0;
}