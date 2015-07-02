#include "wikiobjects.h"
#include "wiki.h"
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




int main(int argc, char **argv)
{
    WikiPtr wiki(new Wiki(new FbySQLiteDB("../var/fby.sqlite3")));
    int imagecount = 0;

    wiki->BeginTransaction();
    for (int arg = 1; arg < argc; ++arg)
    {
        FileFind(argv[arg],
                 [&wiki, &imagecount](fs::directory_iterator dir_iter)
                 {
                     string path(dir_iter->path().string());
                     size_t pos = path.rfind('.');
                     if (pos != string::npos)
                     {
                         if (!strcasecmp(path.c_str() + pos, ".jpg"))
                         {
                             ++imagecount;
                             wiki->LoadJPEGData(path);
                         }
                         else if (!strcasecmp(path.c_str() + pos, ".png"))
                         {
                             ++imagecount;
                             wiki->LoadPNGData(path);
                         }
                     }
                 }
            );
    }
    wiki->EndTransaction();
    cout << "Processed " << imagecount << " image files" << endl;
}

