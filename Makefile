
FUNCS = \
  funcs/count.h funcs/cutgrep.h funcs/file.h funcs/flatten.h funcs/head.h funcs/index.h funcs/math.h

INCLUDE = \
  atom.h command.h deps.h exec.h funcs.h infer.h object.h parse.h tab.h type.h 

SRC = tab.cc 

tab: $(SRC) $(INCLUDE) $(FUNCS)
	g++ -std=c++11 -ggdb -O3 -Wall -Iaxe -lm tab.cc -o tab
