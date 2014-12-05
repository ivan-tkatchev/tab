
FUNCS = \
  funcs/count.h funcs/cutgrep.h funcs/file.h funcs/flatten.h funcs/head.h \
  funcs/index.h funcs/math.h funcs/zip.h funcs/filter.h funcs/sum.h funcs/if.h \
  funcs/sort.h funcs/misc.h funcs/avg.h funcs/array.h funcs/minmax.h funcs/hist.h

INCLUDE = \
  atom.h command.h deps.h exec.h funcs.h infer.h object.h parse.h tab.h type.h 

SRC = tab.cc 

CXX ?= g++

tab: $(SRC) $(INCLUDE) $(FUNCS)
	$(CXX) -std=c++11 -O3 -Wall -Iaxe -lm tab.cc -o tab

README.html:
	echo "<!DOCTYPE html><html><head><style>" > test.html
	cat style.css >> test.html
	echo "</style></head><body>" >> test.html
	cat README.md | markdown_py2 -ohtml4 -x codehilite -x def_list -x fenced_code -x headerid >> test.html
	echo "</body></html>" >> test.html
