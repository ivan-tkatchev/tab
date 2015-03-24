
FUNCS = \
  funcs/count.h funcs/cutgrep.h funcs/file.h funcs/flatten.h funcs/head.h \
  funcs/index.h funcs/math.h funcs/zip.h funcs/filter.h funcs/sum.h funcs/if.h \
  funcs/sort.h funcs/misc.h funcs/avg.h funcs/array.h funcs/minmax.h funcs/hist.h \
  funcs/reverse.h

INCLUDE = \
  atom.h command.h deps.h exec.h funcs.h infer.h object.h parse.h tab.h type.h 

SRC = tab.cc 

CXX ?= g++

tab: $(SRC) $(INCLUDE) $(FUNCS)
	$(CXX) -std=c++11 -O3 -Wall -Iaxe -lm tab.cc -o tab

dist: $(SRC) $(INCLUDE) $(FUNCS)
	$(CXX) -std=c++11 -O3 -Wall -Iaxe -lm tab.cc -static -o tab-linux-x86_64
	strip tab-linux-x86_64

install: tab
	strip tab
	cp tab /usr/local/bin/
