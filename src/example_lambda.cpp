#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

using namespace std;

template <class V> void ForEach(V &v, std::function<void (int i)> f)
{
    for (auto i = v.begin(); i != v.end(); ++i)
    {
        f(*i);
    }
}

int main(int /* argc */, char ** /* argv */)
{
    vector<int> v;
    v.push_back(3);
    v.push_back(5);
    v.push_back(2);
    v.push_back(1);
    v.push_back(7);
    for_each(v.begin(), v.end(), [](int i) { cout << i << endl; });
    cout << "Internal func"<<endl;
    ForEach(v, [](int i) { cout << i << endl; });


}
