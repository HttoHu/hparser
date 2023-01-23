#include <iostream>
#include "../includes/scanner.h"
#include "../includes/production.h"
#include "../includes/ll_parser.h"
// void test()
// {
//     if (HParser::Test::test_first("first1"))
//         std::cout << "Test Okay!\n";
//     else
//         std::cout << "Test failed!\n";
// }

int main()
{
    using namespace HParser;
    using namespace HLex;
    std::string content = HLex::read_file("./test.hgram");
    auto scanner = HLex::scanner(content);
    std::unique_ptr<Context> context = std::make_unique<Context>(scanner);

    LLParser parser(std::move(context), "S");
    parser.gen_ll_tab();
    parser.print_ll_tab();
    std::vector<HLex::Token> vecs;
    vecs.push_back({"int", "123"});
    vecs.push_back({"plus", "+"});
    vecs.push_back({"int", "234"});
    vecs.push_back({"end", "$"});
    auto node = std::move(parser.parse(vecs));
    node->print();
    return 0;
}