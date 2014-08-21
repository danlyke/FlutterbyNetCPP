#include <iostream>
#include <string>
#include <string.h>


using namespace std;

char aStringWithEmbeddedZero[] = "Yo!\0 What up?";

int main(int, char**)
{
    string s(aStringWithEmbeddedZero, sizeof(aStringWithEmbeddedZero) - 1);
    cout << "The size " << s.size() << " should be " << sizeof(aStringWithEmbeddedZero) << " minus one" << endl;

    string s1(s);
    cout << "The size " << s1.size() << " should be " << sizeof(aStringWithEmbeddedZero) << " minus one" << endl;

    return 0;
}
