
MARKDOWN_OPTS = \
  -ohtml4 -x smarty -x codehilite -x def_list -x fenced_code -x headerid -x attr_list -x wikilinks -c opts.json

README.html: README.html.md style.css
	echo "<!DOCTYPE html><html><head><style>" > README.html
	cat style.css >> README.html
	echo "</style></head><title>The tab programming language.</title><body>" >> README.html
	cat README.html.md | markdown_py2 $(MARKDOWN_OPTS) >> README.html
	echo "</body></html>" >> README.html

