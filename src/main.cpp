#include <iostream>
#include "../includes/scanner.h"
#include "../includes/production.h"
void test(){
    if(HParser::Test::test_first("first2"))
        std::cout<<"Test Okay!\n";
    else 
        std::cout<<"Test failed!\n";
}

int main()
{
    std::string content = HLex::read_file("./test.hgram");
    auto scanner = HLex::scanner(content);
    HParser::Context con(scanner);
    con.calc_first();
    for (auto [name, symbol] : con.symb_tab)
    {
        if (symbol->is_terminal())
            continue;
        std::cout << name << ":";
        for (auto item : symbol->firsts)
        {
            std::cout<<con.rsymb_tab[item]<<" ";
        }
        std::cout<<";\n";
    }
    return 0;
}