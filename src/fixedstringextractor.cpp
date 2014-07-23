#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include "fbyregex.h"
#include "fby.h"

using namespace std;
using namespace FbyHelpers;

Regex regex_FbySection("^\\s*\\/\\/\\s*FbySection:\\s*(.*?)\\s*$");

static void EscapeBackslashes(ostream & oscpp, const string &s, size_t start, size_t end)
{
    while (start < end)
    {
        size_t pos = s.find('\\', start);
        if (pos != string::npos && pos < end)
        {
            oscpp << s.substr(start, pos - start);
            oscpp << "\\\\";
            start = pos + 1;
        }
        else
        {
            oscpp << s.substr(start, end - start);
            start = end;
        }
    }
}

static void EscapeQuotes(ostream & oscpp, const string &s, size_t start, size_t end)
{
    while (start < end)
    {
        size_t pos = s.find('"', start);
        if (pos != string::npos && pos < end)
        {
            EscapeBackslashes(oscpp, s, start, pos);
            oscpp << "\\\"";
            start = pos + 1;
        }
        else
        {
            EscapeBackslashes(oscpp, s, start, end);
            start = end;
        }
    }
}


int main(int argc, char **argv)
{
    if (argc != 4)
    {
        cerr << "usage: fixestringextractor prefix infile outbase" << endl;
    }
    string prefix(argv[1]);
    string infile(argv[2]);
    string outbase(argv[3]);
    filebuf fileh, filecpp;

    fileh.open(outbase+".h", ios::out);
    filecpp.open(outbase+".cpp", ios::out);
    ostream osh(&fileh);
    ostream oscpp(&filecpp);
    
    ifstream reader(infile);
    RegexMatch match;

    if (!reader.is_open())
    {
        cerr << "Unable to open ";
        cerr << infile << " for reading" << endl;
    }

    string line;
    string section;
    osh << "#ifndef " << outbase << "_H_INCLUDED" << endl;
    osh << "#define " << outbase << "_H_INCLUDED" << endl;
    oscpp << "#include \"" << outbase << ".h\"" << endl << endl;

    while (getline(reader, line))
    {
        if (regex_FbySection.Match(line, match))
        {
            if (!section.empty())
            {
                oscpp << ";\n\n";
            }
            section = match.Match(1);
            osh << "extern const char *" << prefix << "_" << section << ";" << endl;
            oscpp << "const char *" << prefix << "_" << section << " = \n";
        }
        else
        {
            oscpp << '"';

            EscapeQuotes(oscpp, line, 0, line.length());
            oscpp << "\\n\"" << endl;
        }
    }
    osh << "#endif /* #ifndef " << outbase << "_H_INCLUDED */" << endl;
    oscpp << ";" << endl;
    return 0;
}
