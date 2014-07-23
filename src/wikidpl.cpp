#include "wikidpl.h"
#include <vector>
#include "fbyregex.h"
#include "fbydb.h"
#include "wikiobjects.h"

using namespace std;

static string EscapeBackslashes(const string &s, size_t start, size_t end)
{
    string out;

    while (start < end)
    {
        size_t pos = s.find('\\', start);
        if (pos != string::npos && pos < end)
        {
            out += s.substr(start, pos - start);
            out += "\\\\";
            start = pos + 1;
        }
        else
        {
            out += s.substr(start, end - start);
            start = end;
        }
    }
    return out;
}

static string EscapeSingleQuotes(const string &s, size_t start, size_t end)
{
    string out;

    while (start < end)
    {
        size_t pos = s.find('\'', start);
        if (pos != string::npos && pos < end)
        {
            out += EscapeBackslashes(s, start, pos);
            out += "\\'";
            start = pos + 1;
        }
        else
        {
            out += EscapeBackslashes(s, start, end);
            start = end;
        }
    }
    return out;
}

static string JavaScriptQuote(const string &s)
{
    return EscapeSingleQuotes(s, 0, s.length());
}


const string & DPLNode::Name()
{
    static string name("DPL");
    return name;
}


const string StringOrBlank(map<string,string> attrs, const char *key)
{
    static const string blank("");

    auto entry = attrs.find(string(key));
    if (entry == attrs.end())
        return blank;
    return entry->second;
}


void DPLNode::AsHTML(HTMLOutputter &outputter)
{
    vector<WikiEntryPtr> objs;

    outputter.AddDPLList(StringOrBlank(attrs,"category"),
                         StringOrBlank(attrs,"order"),
                         StringOrBlank(attrs,"pattern"),
                         StringOrBlank(attrs,"count"));
}

