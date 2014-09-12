#include <stdio.h>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <fstream>
#include <iostream>
#include <string.h>

#include "fbyfilefind.h"
#include "fbyregex.h"

using namespace std;
using namespace boost::iostreams ;
namespace fs = boost::filesystem;


Regex regexJheadResolution("JHead Resolution",
                           "Resolution\\s+\\:\\s+(\\d+)\\s*x\\s*(\\d+)");



void FindImageSize(const string &filename,
                   int & /* width */, int & /* height */)
{
    string command("jhead '" + filename + "'");

    cout << command << endl;

    FILE *f = popen(command.c_str(), "r");
    if (NULL == f)
    {
        string errstr("Unable to popen: " + command);
        perror(errstr.c_str());
        return;
    }

    char buffer[256];
    RegexMatch match;
    while (NULL != fgets(buffer, sizeof(buffer) - 1, f))
    {
        string s(buffer);
        if (regexJheadResolution.Match(s, match))
        {
            cout << "Image " << filename << " is " << match.Match(1) << " by " << match.Match(2) << endl;
        }
// << s << endl;
    }

    pclose(f);
    return;
}

int main(int argc, char **argv)
{
    for (int arg = 1; arg < argc; ++arg)
    {
        FileFind(argv[arg],
                 [](fs::directory_iterator dir_iter)
                 {
                     string path(dir_iter->path().filename().string());
                     size_t pos = path.rfind("/");
                     if (pos != string::npos)
                     {
                         path = path.erase(0, pos);
                     }
                     pos = path.rfind('.');
                     if (pos != string::npos
                         && !strcasecmp(path.c_str() + pos, ".jpg"))
                     {
                         int width = -1, height = -1;
                         FindImageSize(dir_iter->path().filename().string(),
                                       width, height);
                     }
                 }
            );
    }
}

