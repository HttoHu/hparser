#include <iostream>
#include "../includes/scanner.h"
#include "../includes/production.h"
#include "../includes/ll_parser.h"
void test()
{
    if (HParser::Test::test_first("first1"))
        std::cout << "Test Okay!\n";
    else
        std::cout << "Test failed!\n";
}

int main()
{
    using namespace HParser;
    std::string content = HLex::read_file("./test.hgram");
    auto scanner = HLex::scanner(content);
    std::unique_ptr<Context> context = std::make_unique<Context>(scanner);

    LLParser parser(std::move(context));
    parser.gen_ll_tab();
    parser.print_ll_tab();
    
    return 0;
}