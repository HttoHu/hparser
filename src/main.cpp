#include <iostream>
#include "../includes/scanner.h"
#include "../includes/production.h"
int main()
{
    std::string content = HLex::read_file("./test.hgram");
    auto scanner = HLex::scanner(content);
    HParser::Context con(scanner);
    con.calc_nullable();

    for (auto item : con.nullable)
    {
        std::cout << con.rsymb_tab[item] << " ";
    }
    return 0;
}