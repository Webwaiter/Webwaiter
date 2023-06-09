#!/usr/bin/env python
import cgi
import os

print("Content-Type: text/html")
print
print("<html><body>")

query_string = os.environ.get("QUERY_STRING", "")

print("<h1>key-value:</h1>")
print("<p>{}</p>".format(cgi.escape(query_string)))

print("</body></html>")
