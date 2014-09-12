#include "fbyimageutils.h"
#include "fbyregex.h"

#include <map>

using namespace std;

/*

File size    : 1909734 bytes
File date    : 2013:09:16 08:25:13
Camera make  : HTC
Camera model : myTouch_4G_Slide
Date/Time    : 2013/08/16 07:10:43
Resolution   : 3264 x 2448
Focal length :  3.7mm
ISO equiv.   : 738
GPS Latitude : N 38d 13m 29.911s
GPS Longitude: W 122d 37m 39.625s
GPS Altitude :  0.00m
JPEG Quality : 90
*/

Regex regexJheadResolution("JHead Resolution",
                           "Resolution\\s+\\:\\s+(\\d+)\\s*x\\s*(\\d+)");

Regex regexJheadAttribute("JHead Attribute",
                          "^(.*?)\\s+\\:\\s+(.*)$");


bool FindJPEGSize(const string &filename,
                  int & width, int & height,
                  map<string,string> &attributes)
{
    string command("jhead '" + filename + "'");

    FILE *f = popen(command.c_str(), "r");
    if (NULL == f)
    {
        string errstr("Unable to popen: " + command);
        perror(errstr.c_str());
        return false;
    }

    char buffer[256];
    RegexMatch match;
    while (NULL != fgets(buffer, sizeof(buffer) - 1, f))
    {
        string s(buffer);
        if (regexJheadResolution.Match(s, match))
        {
            width = stoi(match.Match(1));
            height = stoi(match.Match(2));

//            cout << "Image " << filename << " is " << match.Match(1) << " by " << match.Match(2) << endl;
        }
        else if (regexJheadAttribute.Match(s, match))
        {
            attributes[match.Match(1)] = match.Match(2);
        }
// << s << endl;
    }

    pclose(f);
    return true;
}


string CreateThumbnail(string targetdir, string imagefile, int width)
{
    string imagepath(targetdir + imagefile);
    string destimagepath(targetdir + to_string(width) + "px-" + imagefile);
    string cmd("convert -auto-orient -geometry "
               + to_string(width) + "x" + to_string(width)
               + " '" + imagepath 
               + "' '" + destimagepath + "'");
    ::system(cmd.c_str());
    return destimagepath;
}
