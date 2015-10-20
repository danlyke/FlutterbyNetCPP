#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
// #include "cgicc/HTMLClasses.h"
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <pstreams/pstream.h>

#include "fbydb.h"
#include "fbyregex.h"
#include "wikiobjects.h"
#include "fbystring.h"

#include <vector>
using namespace std;

const char *html_header =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "    <html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
    "\n"
    "    <head>\n"
    "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
    "    <title>Set Status</title>\n"
    "\n"
    "    <script>\n"
    "    function handler(location) {\n"
    "	var lat = document.getElementById(\"lat\");\n"
    "	var lon = document.getElementById(\"lon\");\n"
    "	lat.value = location.coords.latitude;\n"
    "	lon.value = location.coords.longitude;\n"
    "	var posaccuracy = document.getElementById(\"posaccuracy\");\n"
    "	posaccuracy.value = location.coords.accuracy;\n"
    "}\n"
    "navigator.geolocation.getCurrentPosition(handler);\n"
    "</script>\n"
    "    </head>\n"
    "\n"
    "    <body>\n"
    "    <div id=\"page_wrapper\">\n"
    "\n";


int main(int /* argc */, char ** /* argv */, char **env)
{
    cgicc::Cgicc cgi;

    cout << cgicc::HTTPHTMLHeader() << endl;
    cout << html_header;

    cout << "<h1>Environment</h1>\n<ul>";
    for (int i = 0 ; env[i]; ++i)
    {
        cout << "<li>" << env[i] << "</li>\n";
    }

    cout << "</ul>\n";
    cout << "<h1>Params</h1>\n<dl>";
    for (auto in = cgi.getElements().begin(); in != cgi.getElements().end(); ++in)
    {
        cout << "<dt>" << in->getName() << "</dt><dd>"
             << in->getValue() << "</dd>\n";
    }
    cout << "<h1>Files</h1>\n<dl>";
    for (auto f = cgi.getFiles().begin(); f != cgi.getFiles().end(); ++f)
    {
        cout << "<dt>getName</dt>\n";
        cout << "<dd>" << f->getName() << "</dd>\n";
        cout << "<dt>getFilename</dt>\n";
        cout << "<dd>" << f->getFilename() << "</dd>\n";
        cout << "<dt>getDataType</dt>\n";
        cout << "<dd>" << f->getDataType() << "</dd>\n";
        cout << "<dd>" << f->getData() << "</dd>\n";
        cout << "<dt>getDataLength</dt>\n";
        cout << "<dd>" << f->getDataLength() << "</dd>\n";
    }
        
    cout << "</dl>";

    cout << "<h1>Library Environment</h1>\n<dl>\n";
    auto environment(cgi.getEnvironment());

    cout << "<dt>getServerSoftware</dt><dd>"
		<< environment.getServerSoftware() << "</dd>\n";
    cout << "<dt>getServerName</dt><dd>"
		<< environment.getServerName() << "</dd>\n";
    cout << "<dt>getGatewayInterface</dt><dd>"
		<< environment.getGatewayInterface() << "</dd>\n";
    cout << "<dt>getServerProtocol</dt><dd>"
		<< environment.getServerProtocol() << "</dd>\n";
    cout << "<dt>getServerPort</dt><dd>"
		<< environment.getServerPort() << "</dd>\n";
    cout << "<dt>usingHTTPS</dt><dd>"
		<< environment.usingHTTPS() << "</dd>\n";

    cout << "</dl>\n";
    
    string imagename;
    {
        time_t now(time(NULL));
        char buffer[32];

        strftime(buffer,sizeof(buffer),"%F-%T", localtime(&now));
        for (int i = 0; buffer[i]; ++i)
        {
            if (buffer[i] == ':') buffer[i] = '-';
        }
        imagename = string(buffer);
        imagename += ".jpg";
    }

    cout <<
        "<div id=\"charcount\">0</div>\n"
        "<form method=\"post\" action=\"/cgi-bin/cgitest.cgi\" enctype=\"multipart/form-data\">";

    cout << "<form method=\"POST\" enctype=\"multipart/form-data\">";
    cout <<  "Password: ";
    cout <<  "<input name=\"pw\" type=\"text\" value=\"";
    cout << cgi("pw");
    cout << "\">";
    cout <<
        "<br>"
        "<textarea id=\"status\" name=\"status\" rows=\"4\" cols=\"40\"></textarea>"
        "<br>"
        "<input type=\"checkbox\" name=\"twitter\" checked=\"1\">Twitter"
        "<input type=\"checkbox\" name=\"identica\" checked=\"1\">Identica"
        "<input type=\"checkbox\" name=\"facebook\" checked=\"1\">Facebook"
        "<input type=\"checkbox\" name=\"flutterby\" >Flutterby"
        "<br>L/L: <input id=\"lat\" name=\"lat\" size=\"6\" />"
        "<input id=\"lon\" name=\"lon\" size=\"6\"/>"
        "+-: <input id=\"posaccuracy\" name=\"posaccuracy\" size=\"4\" />"
        "<input type=\"button\" name=\"Here\" value=\"here\" onClick=\"navigator.geolocation.getCurrentPosition(handler);\" />"
        "<br>Photo: <input name=\"photofile\" type=\"file\" />"
        "<br>Photo Name: <input name=\"photoname\" size=\"32\" value=\""
         << imagename << "\" />";
    cout << "<br><input type=\"submit\" name=\"Save\" value=\"save\" /></form><hr />";

    cout <<
        "<script type=\"text/javascript\">\n"
        "\n"
        "(function () {\n"
        "var div, txt;\n"
        "div = document.getElementById('charcount');\n"
        "txt = document.getElementById('status');\n"
        "txt.onkeyup = function () {\n"
        "div.innerHTML = txt.value.length;\n"
        "};\n"
        "txt.focus();\n"
        "})();\n"
        "</script>\n"
        "\n</div>\n"
        "</body></html>\n";
}
