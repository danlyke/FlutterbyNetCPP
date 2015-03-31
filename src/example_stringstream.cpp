#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char **argv)
{
    stringstream ss;
    ss << "Arguments are:" << endl;
    for (int i = 0; i < argc; ++argc)
    {
        ss << "  " << i << ": " << argv[i] << endl;
    }
    cout << ss.str();
    return 0;
}
