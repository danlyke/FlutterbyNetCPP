#include <stdio.h>
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    FILE *f = fopen("does not exist", "r");
    if (NULL == f)
    {
        cout << "Open failed" << endl;
    }
    else
    {
        cout << "Open succeeded" << endl;
        fclose(f);
    }

    return 0;
}
