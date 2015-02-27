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
    "    <meta name=\"viewport\" content=\"width=device-width; initial-scale=1.0; maximum-scale=1.0;\">\n"
    "    <link rel=\"apple-touch-icon\" href=\"/iphone/setstatusicon.png\"/>\n"
    "\n"
    "    <style type=\"text/css\">\n"
    "    @import url(\"/iphone/iphone.css\");\n"
    "</style>\n"
    "\n"
    "    <script type=\"text/javascript\" src=\"/iphone/orientation.js\"></script>\n"
    "    <script type=\"text/javascript\">\n"
    "    window.addEventListener(\"load\", function() { setTimeout(loaded, 100) }, false);\n"
    "\n"
    "function loaded() {\n"
    "    document.getElementById(\"page_wrapper\").style.visibility = \"visible\";\n"
    "    window.scrollTo(0, 1); // pan to the bottom, hides the location bar\n"
    "}\n"
    "</script>\n"
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
    "    <body onorientationchange=\"updateOrientation();\">\n"
    "    <div id=\"page_wrapper\">\n"
    "    <h1>Set Status</h1>\n"
    "\n";


int main(int argc, char**argv, char **env)
{
     FbyDBPtr db(FBYNEW FbyPostgreSQLDB("dbname='flutterbynet' user = 'danlyke' password = 'danlyke' host='localhost' "));
    cgicc::Cgicc cgi;

    cout << cgicc::HTTPHTMLHeader() << endl;
    cout << html_header;

    string param_status = cgi("status");
#if 1
    string person_id;
    if (cgi("pw") == "geflertz")
        person_id = "1";
    if (cgi("pw") == "healer61")
        person_id = "2";
#else
    string person_id(db->selectvalue("SELECT id FROM person WHERE password=digest("
                                     + db->Quote(cgi("pw")) + ",'sha512')"));
#endif

    cout << cgicc::HTTPHTMLHeader() << endl;
    cout << html_header;



    if ((!param_status.empty()) && !person_id.empty())
    {
        bool needsrebuild(false);

        string imagename(cgi("photoname"));
        auto ufh(cgi.getFile("photofile"));

        if ((!cgi("photofile").empty())
                && !imagename.empty())
        {
            cout << "<p><b>Filename: " << ufh->getFilename() << "</b></p>\n";
            cout << "<p><b>Imagename: " << imagename << "</b></p>\n";
            string cwd(get_current_dir_name());
            for (size_t i = 0; i < imagename.length(); ++i)
            {
                if (!(isalnum(imagename[i])
                      || imagename[i] == '-'
                      || imagename[i] == '_'
                      || imagename[i] == '.'))
                {
                    imagename.erase(i, i+1);
                    --i;
                }
            }

            string writecmd("/home/danlyke/bin/fby --writeimagefile \"" + imagename + "\"");
            cout <<  "Getting uploaded photo: " << writecmd << "<br>\n";

            redi::opstream outfh(writecmd.c_str());
            if (outfh)
            {
                ufh->writeToStream(outfh);
            }
            else
            {
                cout <<  "Unable to write image<br>\n";
            }
            chdir(cwd.c_str());
            needsrebuild = true;
        }

    
        cout <<  "<b>Posting status: " << HTMLQuote(param_status) << "</b><br>\n";
    
        string param_latitude = cgi("lat");
        string param_longitude = cgi("lon");
        string param_posaccuracy = cgi("posaccuracy");

        string param_twitter_update = cgi("twitter_update");
        string param_flutterby_update = cgi("flutterby_update");
        string param_facebook_update = cgi("facebook_update");
        string param_identica_update = cgi("identica_update");

        if (param_twitter_update != "1") param_twitter_update = "0";
        if (param_flutterby_update != "1") param_flutterby_update = "0";
        if (param_facebook_update != "1") param_facebook_update = "0";
        if (param_identica_update != "1") param_identica_update = "0";


        
        srandom(time(NULL));
        const char xid_chars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
        char xid[13] = "";
        srand(time(NULL));
        for (size_t i = 0; i < sizeof(xid) - 1; ++i)
        {
            xid[i] = xid_chars[random() % sizeof(xid_chars)];
        }
        xid[sizeof(xid) - 1] = '\0';

        vector<string> keys;
        vector<string> values;

        keys.push_back("status");
        values.push_back(param_status);
        keys.push_back("locationset");
        values.push_back((param_latitude.empty() || param_longitude.empty()) ? "0" : "1");

        if (!param_latitude.empty())
        {
            keys.push_back("latitude");
            values.push_back(param_latitude);
        }
        if (!param_longitude.empty())
        {
            keys.push_back("longitude");
            values.push_back(param_longitude);
        }
        if (!param_posaccuracy.empty())
        {
            keys.push_back("posaccuracy");
            values.push_back(param_posaccuracy);
        }

        keys.push_back("flutterby_update");
        values.push_back(param_flutterby_update.empty() ? "0" : "1");
        keys.push_back("twitter_update");
        values.push_back(param_twitter_update.empty() ? "0" : "1");
        keys.push_back("facebook_update");
        values.push_back(param_facebook_update.empty() ? "0" : "1");
        keys.push_back("identica_update");
        values.push_back(param_identica_update.empty() ? "0" : "1");


        keys.push_back("person_id");
        values.push_back(person_id);

        keys.push_back("imagename");
        values.push_back(imagename);
        keys.push_back("xid");
        values.push_back(xid);

        try {
            db->Insert("statusupdate", keys, values);
        } 
        catch(std::exception& e)
        {
            cout << "<b><i>Error: " << e.what() << "</i></b><br />\n";
        }
        catch(  FbyBaseExceptionPtr e)
        {
            cout << "<b><i>Error: ";
            cout << e->file << ":" << e->line << " " << e->Message;
            cout << "</i></b><br />\n";
            return 1;
        }
      

        string recid(db->selectvalue("SELECT CURRVAL(pg_get_serial_sequence('statusupdate', 'id'))"));

        const char *socialMedias[] =
            {
                "flutterby", "facebook", "identica", "twitter",
                NULL
            };
        for (int i = 0; socialMedias[i]; ++i)
        {
            if (!cgi(socialMedias[i]).empty())
            {
                string subsql = "INSERT INTO update_";
                subsql += socialMedias[i];
                subsql += "(statusupdate_id) VALUES (" + recid + ")";
                db->Do(subsql);
            }
        }

        cout <<  "<p><b>Success</b></p>\n";
//        cgi("status" => '');
//        system('/home/danlyke/bin/fby scanimages >& /dev/null &')
//            if $needsrebuild;
    }

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
        "<form method=\"post\" action=\"/cgi-bin/setstatus.cgi\" enctype=\"multipart/form-data\">";

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
