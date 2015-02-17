#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main(int /* argc */, char ** /* argv */)
{
    vector<int> v;
    v.push_back(3);
    v.push_back(5);
    v.push_back(2);
    v.push_back(1);
    v.push_back(7);

    for (auto i = v.begin(); i != v.end(); ++i)
    {
        cout << *i << endl;
    }
}
