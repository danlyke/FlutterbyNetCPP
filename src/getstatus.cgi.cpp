#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
// #include "cgicc/HTMLClasses.h"


#include "fbydb.h"
#include "fbyregex.h"
#include "wikiobjects.h"
#include "fbystring.h"

#include <vector>
using namespace std;




const char *html_header =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
    "<head>\n"
    "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
    "	<title>Status</title>\n"
    "<style type=\"text/css\">@import \"/screen.css\";\n"
    "#e-content { border: solid  1px #000;\n"
    "	background-color: #fdf; }\n"
    "</style>\n"
    "\n"
    "<link rel=\"icon\" href=\"/favicon.ico\" type=\"image/ico\"></link>\n"
    "<script type=\"text/javascript\" src=\"http://maps.google.com/maps/api/js?sensor=false&amp;v=3.6&key=AIzaSyB7waN-qxOPaSUNi58cBpy0Uhknzx0JMPs\"></script>\n"
    "<script type=\"text/javascript\" src=\"/js/fby/maputils.js\"></script>\n"
    "<script type=\"text/javascript\">//<![CDATA[\n"
    "var mapIcons = {};function addLoadEvent(func) {var oldonload = window.onload;if (typeof oldonload == 'function') {window.onload= function() {oldonload();func();};} else {window.onload = func;}}\n"
    "//]]>\n"
    "</script>\n"
    "<link rel=\"stylesheet\" href=\"/js/ol/default/style.css\" type=\"text/css\">\n"
    "</link>\n"
    "<link  rel=\"stylesheet\" href=\"/js/ol/default/google.css\" type=\"text/css\">\n"
    "</link>\n"
    "<script type=\"text/javascript\" src=\"/js/OpenLayers-2.11/OpenLayers.js\">\n"
    "</script>\n"
    "<script type=\"text/javascript\" src=\"/js/OSM_LocalTileProxy.js\">\n"
    "</script>\n"
    "</head>\n"
    "\n"
    "<body>\n"
    "\n"
    "<form method=\"POST\">\n"
    "<input name=\"q\" type=\"text\" /><input type=\"submit\" name=\"go\" value=\"search\" />\n"
    "</form>\n"
    "<div class=\"content\">\n";



int main(int /* argc */, char ** /* argv */, char ** /* env */)
{
    FbyDBPtr db(FBYNEW FbyPostgreSQLDB("dbname='flutterbynet' user = 'danlyke' password = 'danlyke' host='localhost'"));
    int limit = 50;
    cgicc::Cgicc cgi;

    vector<StatusUpdateWithNamePtr> statuses;

    string sql("SELECT statusupdate.*, person.name AS name, person.shortname AS shortname FROM statusupdate, person WHERE statusupdate.person_id=person.id");

    string param_id = cgi("id");
    if (!param_id.empty())
    {
        vector<string> a(split(',', param_id));
        
        sql += " AND (";
        for (auto s = a.begin(); s != a.end(); ++s)
        {
            if (s != a.begin()) sql += " OR ";

            string ss(*s);
            if (ss.substr(0,3) == "id=")
                ss.erase(0,3);
            sql += "xid=" + db->Quote(ss);
        }
        sql += ")";
    }

    string param_user = cgi("user");
    if (!param_user.empty())
    {
        sql += " AND (person.shortname=" + db->Quote(param_user)
            + " OR person.name=" + db->Quote(param_user) + ")";
    }

    string param_q = cgi("q");
    Regex regW("\\W+");
    
    if (!param_q.empty())
    {
        vector<string> a(split(regW, param_q));
        for (auto s = a.begin(); s != a.end(); ++s)
        {
            sql += " AND to_tsvector('english',status) @@ to_tsquery('english',"
                + db->Quote(*s) + ")";
        }
        limit = 50;
    }

    sql += " ORDER BY id DESC LIMIT " + to_string(limit);
    db->Load(&statuses, sql.c_str());


    cout << cgicc::HTTPHTMLHeader() << endl;
    cout << html_header;

    for (auto status = statuses.begin(); status != statuses.end(); ++status)
    {
        cout << "<article class=\"h-entry\"><p><a class=\"p-author h-card\" \n";
        cout << "href=\"http://www.flutterby.net/User:";
        cout << (*status)->shortname;
        cout << "\">";
        cout << (*status)->name;
        cout << "</a>\n<a href=\"/status/id/";
        cout << (*status)->xid;
        cout << "\"><time class=\"dt-published\" datetime=\"";
        cout << TimeToTextDate((*status)->entered);
        cout << "\">";
        cout << TimeToTextDate((*status)->entered);
        cout << "</time></a> &mdash; <small>\n";

        cout << "twitter&nbsp;(" << (*status)->twitter_update << "/" << (*status)->twitter_updated << ") ";
        cout << "facebook&nbsp;(" << (*status)->facebook_update << "/" << (*status)->facebook_updated << ") ";
        cout << "flutterby&nbsp;(" << (*status)->flutterby_update << "/" << (*status)->flutterby_updated << ") ";

        if (!((*status)->twitterid.empty()))
            cout <<  "TwitterID: " << (*status)->twitterid;

        if ((*status)->latitude && (*status)->longitude)
        {
            cout << "&mdash; Lat,Lon:&nbsp;(<span class=\"p-location\"><span class=\"h-geo\"><span class=\"p-latitude\">";
            cout << (*status)->latitude;
            cout << "</span>,<span class=\"p-longitude\">";
            cout << (*status)->longitude;
            cout << "</span></span></span>)";
        }
        cout << "</small></p><div class=\"e-content\">";

        if (!((*status)->imagename.empty()))
        {
            sql = "SELECT * FROM imageinstance WHERE imagename="
                + db->Quote((*status)->imagename)
                + " ORDER BY WIDTH LIMIT 1";
            ImageInstancePtr imageInstance;
            if (db->LoadOne(&imageInstance, sql, false))
            {
                string filename(imageInstance->filename);
                
                size_t pos = filename.find("public_html");
                if (pos != string::npos)
                {
                    filename.erase(0,pos);
                    pos = filename.find('/');
                    if (pos != string::npos)
                        filename.erase(0,pos);
                }
                cout << "<a href=\"/Image:";
                cout << (*status)->imagename;
                cout << "\"><img src=\"";
                cout << filename;
                cout <<"\" width=\"";
                cout << imageInstance->width;
                cout << "\" height=\"";
                cout << imageInstance->height;
                cout << "\" align=left /></a>\n";
            }
        }
        cout << HTMLQuote((*status)->status);
        cout << "<br clear=\"left\" /></div></article>\n";
    }

    cout << "</body>\n</html>\n";
    return 0;
}


