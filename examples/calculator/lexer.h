#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <fstream>

namespace HLex
{
    using std::map;
    using std::vector;
    using char_type = char;

    struct Token
    {
        std::string tag;
        std::string val;
        std::string to_string() const
        {
            return "<" + tag + "," + val + ">";
        }
    };

    class Lexer
    {
    public:
        Lexer(const std::string &con) : content(con), entry(0)
        {
            fin_stat_tab = {{1, "add"}, {2, "lp"}, {3, "mul"}, {4, "num"}, {5, "rp"}, {6, "space"}, {7, "sub"}, {8, "symbol"}};
            tab = {{{'\t', 6}, {' ', 6}, {'(', 2}, {')', 5}, {'*', 3}, {'+', 1}, {'-', 7}, {'0', 4}, {'1', 4}, {'2', 4}, {'3', 4}, {'4', 4}, {'5', 4}, {'6', 4}, {'7', 4}, {'8', 4}, {'9', 4}, {'A', 8}, {'B', 8}, {'C', 8}, {'D', 8}, {'E', 8}, {'F', 8}, {'G', 8}, {'H', 8}, {'I', 8}, {'J', 8}, {'K', 8}, {'L', 8}, {'M', 8}, {'N', 8}, {'O', 8}, {'P', 8}, {'Q', 8}, {'R', 8}, {'S', 8}, {'T', 8}, {'U', 8}, {'V', 8}, {'W', 8}, {'X', 8}, {'Y', 8}, {'Z', 8}, {'a', 8}, {'b', 8}, {'c', 8}, {'d', 8}, {'e', 8}, {'f', 8}, {'g', 8}, {'h', 8}, {'i', 8}, {'j', 8}, {'k', 8}, {'l', 8}, {'m', 8}, {'n', 8}, {'o', 8}, {'p', 8}, {'q', 8}, {'r', 8}, {'s', 8}, {'t', 8}, {'u', 8}, {'v', 8}, {'w', 8}, {'x', 8}, {'y', 8}, {'z', 8}}, {}, {}, {}, {{'0', 4}, {'1', 4}, {'2', 4}, {'3', 4}, {'4', 4}, {'5', 4}, {'6', 4}, {'7', 4}, {'8', 4}, {'9', 4}}, {}, {}, {}, {{'A', 8}, {'B', 8}, {'C', 8}, {'D', 8}, {'E', 8}, {'F', 8}, {'G', 8}, {'H', 8}, {'I', 8}, {'J', 8}, {'K', 8}, {'L', 8}, {'M', 8}, {'N', 8}, {'O', 8}, {'P', 8}, {'Q', 8}, {'R', 8}, {'S', 8}, {'T', 8}, {'U', 8}, {'V', 8}, {'W', 8}, {'X', 8}, {'Y', 8}, {'Z', 8}, {'a', 8}, {'b', 8}, {'c', 8}, {'d', 8}, {'e', 8}, {'f', 8}, {'g', 8}, {'h', 8}, {'i', 8}, {'j', 8}, {'k', 8}, {'l', 8}, {'m', 8}, {'n', 8}, {'o', 8}, {'p', 8}, {'q', 8}, {'r', 8}, {'s', 8}, {'t', 8}, {'u', 8}, {'v', 8}, {'w', 8}, {'x', 8}, {'y', 8}, {'z', 8}}};
            ignore = {"space"};
            keywords = {{"cos", "cos"}, {"sin", "sin"}, {"tan", "tan"}};
            user_defs = {};
        }
        void print_line(const std::string &s, int &pos)
        {
            int cnt = 100;
            while (pos < s.size() && cnt)
            {
                std::cout << s[pos++];
                cnt--;
            }
            std::cout.flush();
        }
        std::vector<Token> lex()
        {
            std::vector<Token> ret;
            int cur_state = entry;
            // to roll back state tag val
            std::vector<std::pair<int, Token>> pos_stac;
            // str pos
            int pos = 0;
            std::string cur_token;
            std::string cur_tag;

            while (pos < content.size())
            {
                if (tab[cur_state].count(content[pos]))
                {
                    cur_state = tab[cur_state][content[pos]];
                    cur_token += content[pos];

                    if (fin_stat_tab.count(cur_state))
                    {
                        if (fin_stat_tab[cur_state] != "")
                            cur_tag = fin_stat_tab[cur_state];

                        pos_stac.clear();
                        pos_stac.push_back({pos + 1, {cur_tag, cur_token}});
                    }
                    pos++;
                }
                else
                {
                    if (pos_stac.empty())
                    {
                        print_line(content, pos);
                        throw std::runtime_error(" LexerGenerator::lex: Lexer Error");
                    }
                    auto [p, tok] = pos_stac.back();
                    // if a symbol is a keyword pr ignore
                    auto val = tok.val;
                    if (keywords.count(val))
                        tok = Token{keywords[val], val};
                    if (user_defs.count(tok.tag))
                    {
                        p -= tok.val.size();
                        tok.val = user_defs[tok.tag](content, p);
                    }
                    if (!ignore.count(tok.tag))
                        ret.push_back(tok);

                    // roll back
                    pos = p;
                    cur_state = entry;
                    cur_token = cur_tag = "";
                    pos_stac.clear();
                }
            }
            if (pos_stac.size())
                ret.push_back(pos_stac.back().second);
            return ret;
        }

    private:
        std::string content;
        int pos = 0;

        std::map<std::string, std::string> keywords;
        std::map<std::string, std::function<std::string(const std::string &, int &)>> user_defs;
        std::set<std::string> ignore;

        int entry;
        map<int, std::string> fin_stat_tab;
        vector<map<char_type, int>> tab;
    };
}