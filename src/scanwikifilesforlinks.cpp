#include <iostream>
#include <string>
#include <map>
#include <boost/filesystem.hpp>

#include "fbyfilefind.h"
#include "fbydb.h"
#include "wiki.h"


int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " inputdir stagingdir" << std::endl;
    }
    else
    {
        WikiPtr wiki(new Wiki(new FbySQLiteDB("../var/fby.sqlite3")));

        wiki->ScanWikiFilesForLinks(argv[1], argv[2]);
    }
    return 0;

}
