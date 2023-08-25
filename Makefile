
FUNCS = \
  funcs/count.h funcs/cutgrep.h funcs/file.h funcs/flatten.h funcs/head.h \
  funcs/index.h funcs/math.h funcs/zip.h funcs/filter.h funcs/sum.h funcs/if.h \
  funcs/sort.h funcs/misc.h funcs/avg.h funcs/array.h funcs/map.h funcs/minmax.h \
  funcs/hist.h funcs/reverse.h funcs/rand.h funcs/time.h funcs/ngram.h \
  funcs/explode.h funcs/uniques.h funcs/url.h funcs/combo.h funcs/unflatten.h

INCLUDE = \
  api.h atom.h command.h deps.h exec.h funcs.h infer.h hash.h object.h optimize.h parse.h tab.h threaded.h type.h 

SRC = tab.cc help.cc

CXX ?= g++
STATIC_LIBS ?= 

tab: $(SRC) $(INCLUDE) $(FUNCS)
	$(CXX) -std=c++11 -O3 -Wall -Iaxe -pthread -lm $(SRC) -o tab
	strip tab

tab-static: $(SRC) $(INCLUDE) $(FUNCS)
	$(CXX) $(STATIC_LIBS) -std=c++11 -O3 -Wall -Iaxe -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -lm -pthread $(SRC) -static -o tab-static
	strip tab-static

install: tab tab-static
	cp tab tab-static /usr/local/bin/

clean:
	rm tab

test:
	cd test; python3 go.py

.PHONY: test
