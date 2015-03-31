#include "fbywikistrings.h"
#include <iostream>


using namespace std;

int main(int /* argc */, char** /* argv */)
{
    string basename("/test/something/somethingelse/A Wiki File.wiki");
    cout << "String test" << endl;
    cout << FilenameToWikiName(basename) << endl; 
    return 0;
}
