#include <string>
#include <iostream>
#include <ctype.h>

using namespace std;

const char *names[] =
{
    "2011-12-25YubaRiverBobbingForApples1.JPG",
    "2011-12-25YubaRiverRoseArloAndDaniel.JPG",
    "2011-12-25BridgeportCoveredBridgePano_hdr.jpg",
    "JakeRunningDrywallScrews.jpg",
    "2011-11-15ShedProgress2.JPG",
    "PlywoodBoatPlans.JPG",
    "SultanaModelHatchLiftRings0.JPG",
    "2012-10-02SCWALogSplittingDemo.jpg",
    "startsWithLowerCase.jpg",
    NULL
};

string InsertSpacesInName(const string &name)
{
    string result;

    if (name.length() == 0)
        return result;

    size_t pos = 1;
    result += name[0];
    char prevChar = name[0];

    while (pos < name.length() && name[pos] != '.')
    {
        if (isalpha(name[pos]))
        {
            if (!isalpha(prevChar))
            {
                    result += ' ';
            }
            else if (isupper(name[pos]) && !isupper(prevChar))
            {
                result += ' ';
            }
        }
        else if (pos && (isalpha(prevChar)))
        {
            result += ' ';
        }

        result += name[pos];
        prevChar = name[pos];
        ++pos;
    }
    return result;
}


int main(int argc, char **argv)
{
    for (int i = 0; names[i]; ++i)
    {
        cout << names[i] << " : " << InsertSpacesInName(string(names[i])) << endl;
    }
    return 0;
}
