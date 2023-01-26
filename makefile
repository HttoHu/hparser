target = bin/hparser
cc = g++ -g -std=c++17

SRC= $(wildcard ./src/*.cpp)
OBJ= $(addprefix obj/,$(patsubst %.cpp,%.o,$(notdir $(SRC))))
BIN= ./bin/hparser

define COM_INS 
@echo "\033[31m compiling... $@ \033[0m"
@$(cc) -g -c $< -o $@
endef

# Dependencies
scanner_dep = scanner.h
production_dep = scanner.h production.h
ll_parser_dep = production.h ast_node.h ll_parser.h

all:$(target)

obj/main.o:$(addprefix ./src/, main.cpp)
	$(COM_INS)
obj/scanner.o:src/scanner.cpp $(addprefix includes/,$(scanner_dep))
	$(COM_INS)
obj/production.o:src/production.cpp $(addprefix includes/,$(production_dep))
	$(COM_INS)
obj/ll_parser.o:src/ll_parser.cpp $(addprefix includes/,$(ll_parser_dep))
	$(COM_INS)
obj/ast_node.o:src/ast_node.cpp 
	$(COM_INS)
$(target):$(OBJ)
	@$(cc) -g $^ -o $(target) -lpthread
	@echo "\033[33m generated success!\033[0m"

.PHONY clean:
clean:
	@rm $(OBJ)
	@rm $(target)
	@rm ./back.zip
run:
	@$(target)

back: 
	zip -r ./back.zip ./*
