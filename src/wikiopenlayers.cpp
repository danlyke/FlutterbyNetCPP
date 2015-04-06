#include "wikiopenlayers.h"
#include <vector>
#include "fbyregex.h"

#include "openlayerstemplate.h"

using namespace std;
bool debug = false;


static string EscapeBackslashes(const string &s, size_t start, size_t end)
{
    string out;

    while (start < end)
    {
        size_t pos = s.find('\\', start);
        if (pos != string::npos && pos < end)
        {
            out += s.substr(start, pos - start);
            out += "\\\\";
            start = pos + 1;
        }
        else
        {
            out += s.substr(start, end - start);
            start = end;
        }
    }
    return out;
}

static string EscapeSingleQuotes(const string &s, size_t start, size_t end)
{
    string out;

    while (start < end)
    {
        size_t pos = s.find('\'', start);
        if (pos != string::npos && pos < end)
        {
            out += EscapeBackslashes(s, start, pos);
            out += "\\'";
            start = pos + 1;
        }
        else
        {
            out += EscapeBackslashes(s, start, end);
            start = end;
        }
    }
    return out;
}

static string JavaScriptQuote(const string &s)
{
    return EscapeSingleQuotes(s, 0, s.length());
}


Regex regex_kml("kml",
                "^(kml)\\:(.*?)(\\n|$)");

Regex regex_gpstrack("gpstrack",
                     "^(gpstrack):(.*?)\\/(.*?)\\/(.*?)(\\/\\n|\n|$)");

Regex regex_dotkml("dotkml",
                   "^(.*\\.kml)(\\n|$)");

Regex regex_georss("georss",
                   "^(georss)\\:(.*)(\\/.*)(\\n|$)");


Regex regex_latloncsv("latloncsv",
                      "^(-?\\d+\\.\\d+)\\,\\s*(-?\\d+\\.\\d+)(\\,\\s*(.*?))(\\n|$)");

Regex regex_latloncsv_no_label("latloncsv no label",
                      "^(-?\\d+\\.\\d+)\\,\\s*(-?\\d+\\.\\d+)(\\n|$)");


Regex regex_label("label",
                  "^([A-Z].*?)\n");;

Regex regex_lat_lon_title("lat_lon_title",
                          "^(-?\\d+\\.\\d+)\\,\\s*(-?\\d+\\.\\d+)\\n([\\w].*?)(\\n+|$)");

Regex regex_zoom_color("zoom & color",
                       "^(\\d+)\\#([0-9A-F][0-9A-F])([0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F])(\\n|$)");

int OpenLayersNode::mapNum = 0;

const string & OpenLayersNode::Name()
{
    static string name("openlayers");
    return name;
}



static void CopyMap(map<string,string> in, map<string,string> &out)
{
    if (debug) cout << "Copying map" << endl;
    for (auto i = in.begin(); i != in.end(); ++i)
    {
        if (debug) cout << "    " << i->first << ": " << i->second << endl;
        out[i->first] = i->second;
    }
}

static void set_default(map<string,string> &attrs, string key, string value)
{
    if (attrs.find(key) == attrs.end())
        attrs[key] = value;
}

void OpenLayersNode::AsHTML(HTMLOutputter &outputter)
{
    static const char * str_width = "width";
    static const char * str_height = "height";
    static const char * str_px = "px";

    attrs[string("mapnum")] = to_string(++mapNum);
    set_default(attrs, str_width, to_string(640));
    set_default(attrs, str_height, to_string(480));

    if (debug) cout << "Outputing OpenLayers node " << mapNum << endl;

    attrs[str_width] = attrs[str_width] + str_px;
    attrs[str_height] = attrs[str_height] + str_px;
    
    {
        map<string, string> vars;

        CopyMap(attrs, vars);

        static const char *prefix =
            "<div class=\"map\" id=\"map$mapnum\" style=\"width: $width; height: $height; direction: ltr;\"></div>"
            "<script type=\"text/javascript\">\n"
            "//<![CDATA[\n";

        outputter.AddString(subst(prefix, vars));
        outputter.AddString(subst(sections_preamble,vars));
    }

    

    bool markerlayer = false;
    const char *buffer = text.data();
    size_t bufferLength = text.length();
 
    while (bufferLength && isspace(*buffer))
    {
        --bufferLength;
        ++buffer;
    }
    if (debug) cout << "Processing line (" << bufferLength << ") '" << string(buffer, bufferLength) << "'" << endl;
    while (bufferLength)
    {
        RegexMatch match;
        bool changed(false);

        if (regex_kml.Match(buffer, bufferLength, match))
        {
            buffer += match.End(0); bufferLength -= match.End(0);

            map<string, string> vars;
            CopyMap(attrs, vars);
            if (debug) cout << "Outputting kml URL '" << match.Match(2) << "'" << endl;
            vars[string("kmlurl")] = match.Match(2);
	    outputter.AddString(subst(sections_KML, vars));
            changed = true;
		} else if (regex_gpstrack.Match(buffer, bufferLength, match)) {
            buffer += match.End(0); bufferLength -= match.End(0);

            map<string, string> vars;
            CopyMap(attrs, vars);
            
            vars[string("fromdate")] = match.Match(2);
            vars[string("todate")] = match.Match(3);
            vars[string("imgoffset")] = match.Match(4);
            if (vars.find("imgoffset") == vars.end())
                vars[string("imgoffset")] = string("14");
            outputter.AddString(subst(sections_gpstracklayer, vars));
            changed = true;
        } else if (regex_dotkml.Match(buffer, bufferLength, match)) 
        {
            buffer += match.End(0); bufferLength -= match.End(0);

            map<string, string> vars;
            CopyMap(attrs, vars);
        
            vars[string("kmlurl")] = match.Match(1);
            outputter.AddString(subst(sections_KML, vars));
			changed = true;
		} else if (regex_georss.Match(buffer, bufferLength, match))
        {
            buffer += match.End(0); bufferLength -= match.End(0);

            map<string, string> vars;
            CopyMap(attrs, vars);
            
			vars[string("georsslayer")] = match.Match(3);
			vars[string("georssurl")] = match.Match(2) + match.Match(3);
			outputter.AddString(subst(sections_GeoRSS, vars));
			changed = true;
		}
        else if (regex_latloncsv.Match(buffer, bufferLength, match))
        {
            if (debug) cout << "Latlon csv " << match.Match(0) << endl;
            buffer += match.End(0); bufferLength -= match.End(0);

            string lat = match.Match(1);
            string lon = match.Match(2);
			string caption;
            string title;

			if (!match.Match(3).empty()) {
                title = match.Match(4);
				caption = "<p><b>" + match.Match(4) + "</b></p>";
			}

			while (regex_label.Match(buffer, bufferLength, match))
            {
                buffer += match.End(0); bufferLength -= match.End(0);

				caption = caption + match.Match(1);
			}

            caption = JavaScriptQuote(caption);
            title = JavaScriptQuote(title);

			if (!markerlayer)
            {
				markerlayer = true;
                map<string, string> vars;
                CopyMap(attrs, vars);
				outputter.AddString(subst(sections_markerlayerbegin, vars));
			}

            map<string, string> vars;
            CopyMap(attrs, vars);

			vars[string("caption")] = caption;
			vars[string("title")] = title;
			vars[string("lat")] = lat;
			vars[string("lon")] = lon;
			outputter.AddString(subst(sections_marker, vars));
			changed = true;
		}
		else if (regex_lat_lon_title.Match(buffer, bufferLength, match))
		{
            if (debug) cout << "Latlon title " << match.Match(0) << endl;
            buffer += match.End(0); bufferLength -= match.End(0);

			string lat = match.Match(1);
			string lon = match.Match(2);
			string caption;
			string title;

            if (match.Count() > 3 && match.Start(3) != match.End(3))
            {
				title = match.Match(3);
				caption = "<p><b>" + match.Match(3) + "</b></p>";
			}
			
			while (regex_label.Match(buffer, bufferLength, match))
            {
                buffer += match.End(0); bufferLength -= match.End(0);

				caption = caption + match.Match(1);
			}
			
            caption = JavaScriptQuote(caption);

            map<string, string> vars;
            CopyMap(attrs, vars);
			
			if (markerlayer)
            {
				markerlayer = true;
				outputter.AddString(subst(sections_markerlayerbegin, vars));
			}
        
        
            vars[string("caption")] = caption;
            vars[string("title")] = title;
			vars[string("lat")] = lat;
			vars[string("lon")] = lon;
			outputter.AddString(subst(sections_marker, vars));
			changed = true;
		}
        else if (regex_zoom_color.Match(buffer, bufferLength, match))
		{
            buffer += match.End(0); bufferLength -= match.End(0);

			string zoomq = match.Match(1);
			string huh = match.Match(2);
			string color = match.Match(3);
            map<string, string> vars;
            CopyMap(attrs, vars);

			vars[string("zoomq")] = zoomq;
			vars[string("huh")] = huh;
			vars[string("color")] = color;

			outputter.AddString(subst(sections_vectorlayerpreamble, vars));
            
			while (regex_latloncsv_no_label.Match(buffer, bufferLength, match))
			{
                buffer += match.End(0); bufferLength -= match.End(0);

				vars[string("lat")] = match.Match(1);
				vars[string("lon")] = match.Match(2);
				outputter.AddString(subst(sections_vectorlayerpoint, vars));
			}
            vars.erase(vars.find("lat"));
            vars.erase(vars.find("lon"));
			outputter.AddString(subst(sections_vectorlayerpost, vars));
			changed = true;
		}
        // THIS LOOKS WRONG!
        else if (regex_latloncsv.Match(buffer, bufferLength, match))
		{
            buffer += match.End(0); bufferLength -= match.End(0);

			string zoomq = match.Match(1);
			string huh = match.Match(2);
			string color = "#758BC5";
            map<string, string> vars;
            CopyMap(attrs, vars);

			vars[string("zoomq")] = zoomq;
			vars[string("huh")] = huh;
			vars[string("color")] = color;

            if (debug) cout << "Outputting latloncsv for " << match.Match(0) << endl;

			outputter.AddString(subst(sections_vectorlayerpreamble, vars));
			while (regex_latloncsv_no_label.Match(buffer, bufferLength, match))
			{
                buffer += match.End(0); bufferLength -= match.End(0);

				vars[string("lat")] = match.Match(1);
				vars[string("lon")] = match.Match(2);
				outputter.AddString(subst(sections_vectorlayerpoint, vars));
                vars.erase(vars.find(string("lat")));
                vars.erase(vars.find(string("lon")));
			}
			outputter.AddString(subst(sections_vectorlayerpost, vars));
			changed = true;
		}

        if (!changed)
        {
            cout << "Parse failure: '" << string(buffer, bufferLength) << "'" << endl;
            break;
        }
    }


    {
        map<string, string> vars;
        CopyMap(attrs, vars);
        outputter.AddString(subst(sections_post, vars));
    }

    static const char *suffix =
        "//]]>\n"
        "</script>\n";
    outputter.AddString(suffix);
}

