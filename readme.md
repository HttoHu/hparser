## HParser

A simple parser supports LL(1) (finished) and LR(1)(in the plan) parser..

## principle
LL(1) Parser:
1. Calculate First-set and follow-set and nullable-set by fixed point algorithm (production.cpp)

2. Calculate every productions' first-set, and generate the LL(1) parsing table.

3. Parsing procedure.
   * Firstly push the destination non-terminal symbol to the parsing stack 
   
   * If the top of the parsing stack is non-terminal symbol, get the matched production by LL(1) table(By cur non-terminal symbol on the stack and current token to find the production). Push the correspondent symbols to a stack.
   
   * If the top of the parsing stack is terminal, read a token and check if they are same type and pop it (if they are different the throw an error).
   
4. Some production ajustments algorithm
   * Because LL(1) parsing cannot process some productions such as left recursive productions, left common factors productions. We need to transform them as the equivalent productions at first.
   
   * Kill left common factors is very simple and intuitive, for example
   
     ```
     =========INPUT========
     A	-> a b c
     	-> a b
         -> a n
     =========OUTPUT==========
     A  -> a A'
     A' -> b A''
        -> n
     A'' -> c
         -> epision
     ```
     The recovery is also every simple:d push the generated nodes to his parent.. 
    * Kill left recursive : read `./docs/alg/kill-left-recursive.md` (Chinese) to learn more. and recovery the AST after kill the left recursive 
   
     
   
     