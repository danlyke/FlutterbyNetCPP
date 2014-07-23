#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include <iostream>
#include <fstream>
#include <map>

#include "wiki.h"
#include "regexparser.h"


using namespace std;



int main(int argc, char** argv)
{
    setlocale(LC_ALL, ""); /* Use system locale instead of default "C" */
    if (argc <= 2) {
        fprintf(stderr, "Usage: %s inputfile[s] outputdir\n", argv[0]);
        return 1;
    }

    WikiPtr wiki(FBYNEW Wiki);
    wiki->BeginTransaction();
    for (int arg = 1; arg < argc - 1; ++arg)
    {
        wiki->ParseAndOutputFile(argv[arg], argv[argc-1]);
    }
    wiki->EndTransaction();

    return 0;
}
