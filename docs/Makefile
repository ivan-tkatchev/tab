
MARKDOWN_OPTS = \
  -ohtml4 -x smarty -x codehilite -x def_list -x fenced_code -x toc -x attr_list -x wikilinks -x tables -c opts.json

all: README.html examples.html api.html

#.SUFFIXES: html.md .html

%.html: %.html.md style.css
	echo "<!DOCTYPE html><html><head><style>" > $@
	cat style.css >> $@
	echo "</style></head><title>The tab programming language.</title><body>" >> $@
	cat $< | markdown_py2 $(MARKDOWN_OPTS) >> $@
	echo "</body></html>" >> $@

#README.html: README.html.md style.css
#	echo "<!DOCTYPE html><html><head><style>" > README.html
#	cat style.css >> README.html
#	echo "</style></head><title>The tab programming language.</title><body>" >> README.html
#	cat README.html.md | markdown_py2 $(MARKDOWN_OPTS) >> README.html
#	echo "</body></html>" >> README.html

