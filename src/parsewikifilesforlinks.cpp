#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <iostream>
#include <fstream>
#include <map>

#include "wiki.h"
#include "fbyregexparser.h"


using namespace std;



int main(int argc, char** argv)
{
    setlocale(LC_ALL, ""); /* Use system locale instead of default "C" */
    if (argc < 1) {
        fprintf(stderr, "Usage: %s inputfile[s]\n", argv[0]);
        return 1;
    }
    WikiPtr wiki(FBYNEW Wiki(FBYNEW FbySQLiteDB("../var/fby.sqlite3")));

    wiki->BeginTransaction();
    for (int arg = 1; arg < argc; ++arg)
    {
        wiki->ScanWikiFileForLinks(argv[arg]);
    }
    wiki->EndTransaction();

    return 0;
}
