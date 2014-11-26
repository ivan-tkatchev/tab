
SRC = tab.cc atom.h command.h deps.h exec.h file.h funcs.h infer.h object.h parse.h tab.h type.h 

tab: $(SRC)
	g++ -std=c++11 -ggdb -O3 -Wall -Iaxe -lm tab.cc -o tab
