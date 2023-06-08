#!/usr/bin/env python
import cgi
import os

form = cgi.FieldStorage()

upload_dir = os.getenv('DOCUMENT_ROOT')

if 'fileToUpload' in form:
    fileitem = form['fileToUpload']

    if fileitem.filename:
        filename = os.path.join(upload_dir, os.path.basename(fileitem.filename))
        
        with open(filename, 'wb') as file:
            file.write(fileitem.file.read())
        
        print("Content-type: text/html")
        print
        print("<html>")
        print("<head>")
        print("<title>File Upload</title>")
        print("</head>")
        print("<body>")
        print("<h2>File uploaded successfully.</h2>")
        print("</body>")
        print("</html>")
    else:
        print("Content-type: text/html")
        print
        print("<html>")
        print("<head>")
        print("<title>File Upload</title>")
        print("</head>")
        print("<body>")
        print("<h2>No file selected for upload.</h2>")
        print("</body>")
        print("</html>")
else:
    print("Content-type: text/html")
    print
    print("<html>")
    print("<head>")
    print("<title>File Upload</title>")
    print("</head>")
    print("<body>")
    print("<h2>File upload form</h2>")
    print("<form action='/upload' method='POST' enctype='multipart/form-data'>")
    print("<input type='file' name='fileToUpload' id='fileToUpload'>")
    print("<input type='submit' value='Upload'>")
    print("</form>")
    print("</body>")
    print("</html>")
