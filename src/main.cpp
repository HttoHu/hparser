#include <iostream>
#include "../includes/scanner.h"
#include "../includes/production.h"
int main()
{
    std::string content = HLex::read_file("./test.hgram");
    auto scanner = HLex::scanner(content);
    HParser::Context con(scanner);
    con.print();
    return 0;
}