#include <functional>
#include "parser.h"
#include <cmath>

namespace Calc
{
    using HParser::ASTNodePtr;
    using std::map;
    using std::string;
    using OBJ = double;
    using eval_type = std::function<OBJ(ASTNodePtr &)>;

    map<string, eval_type> eval_tab;

    void insert_eval(const string &node, eval_type func)
    {
        eval_tab.insert({node, func});
    }
    OBJ call(const string &node, ASTNodePtr &p)
    {
        if(!eval_tab.count(node))
        {
            std::cout<<"Unknown symbol "<< node<<std::endl;
        }
        return eval_tab[node](p);
    }
    void init()
    {
        /*
        [E]:[E]<add>[T]
        |[E]<sub>[T]
        |[T]
        */
        insert_eval("E", [](ASTNodePtr &p) -> OBJ
                    {
            if(p->ch_size() == 1)
                return call("T",p->front());
            else
            {
                OBJ L = call("E",(*p)[0]);
                OBJ R = call("T",(*p)[2]);
                auto op = (*p)[1]->type;
                if(op=="add")
                    return L+R;
                else 
                    return L-R;    

            } });
        /*
        [T]:[T]<mul>[F]
        |[T]<div>[F]
        |[F]
        ;
        */
        insert_eval("T", [](ASTNodePtr &p) -> OBJ
                    {
            if(p->ch_size() == 1)
                return call("F",p->front());
            else
            {
                OBJ L = call("T",(*p)[0]);
                OBJ R = call("F",(*p)[2]);
                auto op = (*p)[1]->type;
                if(op=="mul")
                    return L*R;
                else 
                    return L/R;    
            } });
        /*
        [F]:<num>
        |<lp>[E]<rp>
        |[Func]
        ;
        */
        insert_eval("F", [](ASTNodePtr &p) -> OBJ
                    {
            if(p->ch_size() == 1)
            {
                if(p->front()->type=="num")
                    return stoi(std::get<string>(p->front()->data));
                else 
                    return call("Func",p->front());
            }
            else
            {
                return call("E",(*p)[1]);
            } });
        /*
        [Func]:<sin><lp>[E]<rp>
            |<cos><lp>[E]<rp>
            |<tan><lp>[E]<rp>
            ;
        */
        insert_eval("Func", [](ASTNodePtr &p) -> OBJ
                    {
            auto op = p->front()->type;
            auto arg = call("E", (*p)[2]);
            if (op == "sin")
                return sin(arg);
            else if(op == "cos")
                return cos(arg);
            else 
                return tan(arg);
        });
    }
}