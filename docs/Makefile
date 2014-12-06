
README.html: README.html.md style.css
	echo "<!DOCTYPE html><html><head><style>" > README.html
	cat style.css >> README.html
	echo "</style></head><body>" >> README.html
	cat README.html.md | markdown_py2 -ohtml4 -x codehilite -x def_list -x fenced_code -x headerid >> README.html
	echo "</body></html>" >> README.html

