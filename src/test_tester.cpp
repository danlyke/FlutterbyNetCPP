#include <iostream>
#include <map>
#include <string>
#include <stdlib.h>

using namespace std;

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        string basename(argv[i]);
        size_t pos = basename.rfind(".");
        if (string::npos != pos)
        {
            basename = basename.erase(pos);
        }
        string s("./flextest < " + basename + ".txt > " + basename + ".tmp");

        int ret;
        ret = system(s.c_str());
        if (ret)
            exit(ret);
        s = "diff " + basename + ".tmp " + basename + ".out";

        ret = system(s.c_str());
        if (ret)
        {
            cerr << "Diff failed on " << basename << endl;
            exit(ret);
        }
    }
    exit(0);
}
