#include <iostream>
#include "fbyregex.h"

using namespace std;

int main(int argc, char **argv)
{
    string s("A very  long string that-has-some hyphenated\.");
    Regex regW("\\W+");
    vector<string> a(split(regW, s));
                     
    for (auto i = a.begin(); i != a.end(); ++i)
    {
        cout << "'" << *i << "'" << endl;
    }
    return 0;
}
