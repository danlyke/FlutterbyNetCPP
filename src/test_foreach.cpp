#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;

void PutString(const string &s)
{
    cout << s << endl;
}

const char *stuff[] =
{
    "this",
    "that",
    "the other"
    "thing",
    NULL
};

int main(int argc, char **argv)
{
    vector<string> a;
    for (int i = 0; stuff[i]; ++i)
    {
        string s(stuff[i]);
        a.push_back(s);
    }
    for_each(a.begin(), a.end(), [](string s){ cout << s << endl; });
    return 0;
}
