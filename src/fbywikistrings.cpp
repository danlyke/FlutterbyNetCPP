#include "fbywikistrings.h"
#include "fbyregex.h"

using namespace std;

std::string VarSubst(const std::string &input,
                     std::map<std::string,std::string> vars)
{
    string result;
    size_t start(0);
    size_t length(input.length());
    RegexMatch match;
    Regex reg("^(.*?)\\$(\\w+)");
    while (reg.Match(input.data() + start, length, match))
    {
        result += match.Match(1);
        string var = match.Match(2);
        auto repl = vars.find(var);
        if (repl != vars.end())
            result += repl->second;
        else
            result += "$" + var;
        start += match.End(2);
        length -= match.End(2);
    }
    result += string(input.data() + start, length);
    return result;
}


string NormalizeWikiName(const string &inputname)
{
    string filename(inputname);
    bool collapse = 0;

    for (size_t i = 0; i < filename.length(); ++i)
    {
        if (filename[i] == ':')
        {
            collapse = 1;
        }
        else if (isspace(filename[i]))
        {
            if (collapse)
            {
                filename.erase(i, 1);
            }
            else if (filename[i] != ' ')
            {
                filename[i] = ' ';
            }
            collapse = 1;
        }
        else
        {
            collapse = 0;
        }
    }
    return filename;
}

string FilenameToWikiName(const string &inputname)
{
    size_t slashpos(inputname.rfind('/'));
    size_t dotpos(inputname.rfind('.'));
    if (slashpos == string::npos)
        slashpos = 0;
    else
        slashpos++;
    if (dotpos == string::npos) dotpos = inputname.length();
    string wikiname(inputname.substr(slashpos, dotpos - slashpos));
    return NormalizeWikiName(wikiname);
}

string NormalizeWikiNameToFilename(const string &wikiname)
{
    string filename(wikiname);
    for (size_t i = 0; i < filename.length(); ++i)
    {
        if (isspace(filename[i]))
        {
            filename[i] = '_';
        }
    }
    return filename;
}

