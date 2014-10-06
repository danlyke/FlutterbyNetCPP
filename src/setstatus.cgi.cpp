#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
// #include "cgicc/HTMLClasses.h"


#include "fbydb.h"
#include "fbyregex.h"
#include "wikiobjects.h"
#include "stringutil.h"

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

     FbyDBPtr db(FBYNEW FbyPostgreSQLDB("dbname='flutterbynet' user = 'danlyke' password = 'danlyke'"));
    int limit = 50;
    cgicc::Cgicc cgi;

    string param_status = cgi("status");

    if ((!param_status.empty()) && cgi("pw") eq 'geflertz')
    {
        string imagename;

        bool needsrebuild(false);

        if ((!cgi("photofile").empty())
            && (!my $ufh = $cgi->upload('photofile'.empty()))
            && (!cgi("photoname").empty())
            && cgi("photoname") != "")
        {
            string cwd(get_current_dir_name());
            imagename = cgi("photoname");
            for (i = 0; i < imagename.length(); ++i)
            {
                if (!(isalnum(imagename[i])
                      || imagename[i] == '-'
                      || imagename[i] == '_'))
                {
                    imagename.erase(i, i+1);
                }
            }

            string writecmd("/home/danlyke/bin/fby writeimagefile \"" + imagename + "\"");
            cout <<  "Getting uploaded photo: $writecmd<br>\n";
            FILE *outfh(popen(writecmd(imagename.c_str()), "w"));
            if (outfh)
            {
                FormFile formFile;
                formFile.writeToStream(outfh);
                fclose(outfh);
            }
            else
            {
                cout <<  "Unable to write image<br>\n";
            }
            chdir(cwd.c_str());
            needsrebuild = true;
        }

    
        cout <<  "<b>Posting status: param_status</b><br>\n";
    
        string param_latitude = cgi("lat");
        string param_longitude = cgi("lon");
        string posaccuracy = cgi("posaccuracy");


        const char xid_chars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
        const char xid[13];
        for (int i = 0; i < sizeof(xid) - 1; ++i)
        {
            xid[i] = xid_chars(rand() % sizeof(xid_chars));
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
        values.push_back(param_flutterby_update);
        keys.push_back("twitter_update");
        values.push_back(param_twitter_update);
        keys.push_back("facebook_update");
        values.push_back(param_facebook_update);
        keys.push_back("identica_update");
        values.push_back(param_identica_update);

        keys.push_back("person_id");
        values.push_back(param_person_id);

        keys.push_back("imagename");
        values.push_back(param_imagename);
        keys.push_back("xid");
        values.push_back(param_xid);

        db->Insert("statusupdate", keys, values);

        @a = $dbh->selectrow_array("SELECT CURRVAL(pg_get_serial_sequence('statusupdate', 'id'))");
        string recid = $a[0];
        foreach ('flutterby', 'facebook', 'identica', 'twitter')
        {
            if (defined($cgi->param($_)))
            {
                string subsql = "INSERT INTO update_$_ (statusupdate_id) VALUES ($recid)";
                $dbh->do($subsql)
                            || cout <<  "<p><b>$dbh::errstr</b>: $subsql</p>\n";
            }
        }

        cout <<  "<p><b>Success: $sql</b></p>\n";
        cgi("status" => '');
        $dbh->disconnect();
        system('/home/danlyke/bin/fby scanimages >& /dev/null &')
            if $needsrebuild;
    }

    string imagename;
    {
        my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) =
            localtime(time);
        $imagename = scout << f("%4.4d-%2.2d-%2.2d-%2.2d-%2.2d.jpg",
                             $year + 1900, $mon + 1, $mday, $hour, $min);
    }

    cout <<  <<EOF;
    <div id="charcount">0</div>
         <form method="post" action="/cgi-bin/setstatus.pl" enctype="multipart/form-data">
         EOF
         cout <<  $cgi->start_form;
    cout <<  "Password: ". $cgi->textfield(-name => 'pw', -default => cgi("pw"));
    cout <<  <<EOF;
    <br>
         <textarea id="status" name="status" rows="4" cols="40"></textarea>
         <br>
         <input type="checkbox" name="twitter" checked="1">Twitter
         <input type="checkbox" name="identica" checked="1">Identica
         <input type="checkbox" name="facebook" checked="1">Facebook
         <input type="checkbox" name="flutterby" >Flutterby
         <br>L/L: <input id="lat" name="lat" size="6" />
         <input id="lon" name="lon" size="6"/>
         +-: <input id="posaccuracy" name="posaccuracy" size="4" />
         <input type="button" name="Here" value="here" onClick="navigator.geolocation.getCurrentPosition(handler);" />
         <br>Photo: <input name="photofile" type="file" />
         <br>Photo Name: <input name="photoname" size="32" value="$imagename" />
         EOF
         cout <<  "<br>".$cgi->submit('Save','save');
    cout <<  $cgi->end_form;
    cout <<  $cgi->hr;
    cout <<  <<EOF;
    <script type="text/javascript">

         (function () {
             var div, txt;
             div = document.getElementById('charcount');
             txt = document.getElementById('status');
             txt.onkeyup = function () {
                 div.innerHTML = txt.value.length;
             };
             txt.focus();
         })();
    </script>
          EOF

          cout <<  "\n</div>\n";
    cout <<  $cgi->end_html();
    cout <<  "\n";
#endif
}
