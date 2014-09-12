#include <iostream>
#include <string>
#include <map>
#include <boost/filesystem.hpp>

#include "fbyfilefind.h"
#include "fbydb.h"
#include "wiki.h"


int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " stagingdir" << std::endl;
    }
    else
    {
        WikiPtr wiki(FBYNEW Wiki(FBYNEW FbySQLiteDB("../var/fby.sqlite3")));
        wiki->RebuildDirtyFiles(argv[1]);
    }
    return 0;

}
