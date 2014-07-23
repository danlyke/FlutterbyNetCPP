#include <iostream>
#include <string>
#include <map>
#include <boost/filesystem.hpp>

#include "filefind.h"
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
        WikiPtr wiki(FBYNEW Wiki);
        wiki->RebuildDirtyFiles(argv[1]);
    }
    return 0;

}
