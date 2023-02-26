#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include "../includes/scanner.h"
#include "../includes/production.h"
#include "../includes/ll_parser.h"

char buf[1024];
const int BUF_LEN = 1024;

std::string cur_dir(char *path)
{
    int len = strlen(path);
    int end_pos = len - 1;
    while (end_pos && path[end_pos] != '\\' && path[end_pos] != '/')
        end_pos--;
    return std::string(path, path + end_pos);
}
void gen_code(int argc, char **argv)
{
    using namespace HParser;
    using namespace HLex;
    if (argc != 4)
    {
        std::cerr << "invalid argument usage: hparser [input][output][start symbol]\n";
        exit(1);
    }
    std::string exe_path = cur_dir(argv[0]);
    std::string rule_file = HLex::read_file(argv[1]);
    std::string output_path = argv[2];
    std::string start_symbol = argv[3];

    auto scanner = HLex::scanner(rule_file);

    std::unique_ptr<Context> context = std::make_unique<Context>(scanner);

    context->kill_left_commmon_factor();
    context->kill_left_recursive();

    LLParser parser(std::move(context), start_symbol);
    parser.gen_ll_tab();

    std::ofstream ofs(output_path);

    std::string template_str = read_file(exe_path + "/htemp/hparser.txt");
    ofs << parser.gen_parser_code(template_str) << "\n";
    ofs.close(); 
}
void test()
{
    using namespace HParser;
    using namespace HLex;

    const std::string temp_file = HLex::read_file("./template/template.txt");
    const std::string rule_file = HLex::read_file("./test.hgram");
    auto scanner = HLex::scanner(rule_file);
    std::unique_ptr<Context> context = std::make_unique<Context>(scanner);

    context->kill_left_commmon_factor();
    context->kill_left_recursive();
    context->print();
    LLParser parser(std::move(context), "S");
    parser.gen_ll_tab();
    std::vector<Token> tokens = {
        {"int", "32"}, {"plus", "+"}, {"int", "64"}, {"end", "$"}};
    // auto node = parser.parse(tokens);
    // adjust_ast(node);
    std::ofstream ofs("./test/parser.h");
    ofs << parser.gen_parser_code(temp_file);
    // node->print();
}
int main(int argc, char **argv)
{
    test();
    // gen_code(argc, argv);
    return 0;
}