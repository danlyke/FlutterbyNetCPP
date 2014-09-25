#include "fbydb.h"
#include "fbyregex.h"
#include "wikiobjects.h"
#include "stringutil.h"
using namespace std;


const char *html_header =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
    "<head>\n"
    "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
    "	<title>Status</title>\n"
    "<style type=\"text/css\">\@import \"/screen.css\";\n"
    "#e-content { border: solid  1px #000;\n"
    "	background-color: #fdf; }\n"
    "</style>\n"
    "\n"
    "<link rel=\"icon\" href=\"/favicon.ico\" type=\"image/ico\"></link>\n"
    "<script type=\"text/javascript\" src=\"http://maps.google.com/maps/api/js?sensor=false&v=3.6&key=AIzaSyB7waN-qxOPaSUNi58cBpy0Uhknzx0JMPs\"></script>\n"
    "<script type=\"text/javascript\" src=\"js/fby/maputils.js\"></script>\n"
    "<script type=\"text/javascript\">//<![CDATA[\n"
    "var mapIcons = {};function addLoadEvent(func) {var oldonload = window.onload;if (typeof oldonload == 'function') {window.onload= function() {oldonload();func();};} else {window.onload = func;}}\n"
    "//]]>\n"
    "</script>\n"
    "<link rel=\"stylesheet\" href=\"js/ol/default/style.css\" type=\"text/css\">\n"
    "</link>\n"
    "<link  rel=\"stylesheet\" href=\"js/ol/default/google.css\" type=\"text/css\">\n"
    "</link>\n"
    "<script type=\"text/javascript\" src=\"js/OpenLayers-2.11/OpenLayers.js\">\n"
    "</script>\n"
    "<script type=\"text/javascript\" src=\"js/OSM_LocalTileProxy.js\">\n"
    "</script>\n"
    "</head>\n"
    "\n"
    "<body>\n"
    "\n"
    "<form method=\"POST\">\n"
    "<input name=\"q\" type=\"text\"><input type=\"submit\" name=\"go\" value=\"search\" />\n"
    "</form>\n"
    "<div class=\"content\">\n";



int main(int argc, char**argv, char **env)
{
    FbyDBPtr db(FBYNEW FbyPostgreSQLDB("dbname='flutterbynet' user = 'danlyke' password = 'danlyke'"));
    int limit = 50;

    vector<StatusUpdateWithNamePtr> statuses;

    string sql("SELECT statusupdate.*, person.name AS name FROM statusupdate, person WHERE statusupdate.person_id=person.id");

    if ($cgi->param("id"))
    {
        vector<string> a(split(cgiparam("q"), ","));
        
        sql += " AND (";
        for (auto s = a.begin(); s != a.end(); ++s)
        {
            if (s != a.begin()) sql += " OR ";
            sql += "xid=" + dbh->quote(s);
        }
    }
    if ($cgi->param("user"))
    {
        sql += " AND (person.shortname=" + dbh->quote(cgi->param("user"))
            + " OR person.name=" + dbh->quote(cgi->param("user")) + ")";
    }
    elsif (cgi->param("q"))
    {
        vector<string> a(split /\W+/, cgi->param("q"));
        for (auto s = a.begin(); s != a.end(); ++s)
        {
            sql += " AND to_tsvector('english',status) @@ to_tsquery('english',"
                + $dbh->quote($_).")";
        }
        limit = 50;
    }

    sql += " ORDER BY id DESC LIMIT " + to_string(limit);
    db->Load(statuses, sql.c_str());

    cout <<
    return 0;
}


