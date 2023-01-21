target = bin/hparser
cc = g++ -g -std=c++17

SRC= $(wildcard ./src/*.cpp)
OBJ= $(addprefix obj/,$(patsubst %.cpp,%.o,$(notdir $(SRC))))
BIN= ./bin/hparser

define COM_INS 
@echo "\033[31m 正在编译$@ \033[0m"
@$(cc) -g -c $< -o $@
endef

# Dependencies
scanner_dep = scanner.h
production_dep = scanner.h production.h

all:$(target)

obj/main.o:$(addprefix ./src/, main.cpp)
	$(COM_INS)
obj/scanner.o:src/scanner.cpp $(addprefix includes/,$(scanner_dep))
	$(COM_INS)
obj/production.o:src/production.cpp $(addprefix includes/,$(production_dep))
	$(COM_INS)

$(target):$(OBJ)
	@$(cc) -g $^ -o $(target) -lpthread
	@echo "\033[33m 生成完毕\033[0m"

.PHONY clean:
clean:
	@rm $(OBJ)
	@rm $(target)
	@rm ./back.zip
run:
	@$(target)

back: 
	zip -r ./back.zip ./*
