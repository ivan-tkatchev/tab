
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

README.html: README.html.md style.css
	echo "<!DOCTYPE html><html><head><style>" > README.html
	cat style.css >> README.html
	echo "</style></head><body>" >> README.html
	cat README.html.md | markdown_py2 -ohtml4 -x codehilite -x def_list -x fenced_code -x headerid >> README.html
	echo "</body></html>" >> README.html
