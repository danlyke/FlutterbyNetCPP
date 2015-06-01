#include "fby.h"
#include "fbydb.h"
#include "wikiobjects.h"

#include <iostream>



using namespace std;

const char *testDates[] =
{
    "1970-01-01 00:00:00+0000",
    "2012-02-05 00:00:00+0000",
    "1971-02-05 00:00:00+0000",
    "2000-02-05 00:00:00+0000",
    "2012-02-05 12:00:00+0000",
    "1971-02-05 12:00:00+0000",
    "2000-02-05 12:00:00+0000",
    "2012-02-05 14:00:00+0000",
    "1971-02-05 14:00:00+0000",
    "2000-02-05 14:00:00+0000",
    NULL
};

int main(int /* argc */, const char * const * /* argv */)
{
    {
        time_t t = 0;
        time_t t1 = mktime(gmtime(&t));
        time_t t2 = mktime(localtime(&t));
        time_t td = t2 - t1;
        cout << "Diff " << td << " in hours " << (td / 3600) << endl;
        cout << TimeToTextDate(0) << endl;
    }

    for (int i = 0; testDates[i]; ++i)
    {
        time_t t(TextDateToTime(testDates[i]));
        string ts(TimeToTextDate(t));
        if (ts != string(testDates[i]))
        {
            cout << "Warning: time " << ts
                 << " does not match " << testDates[i] 
                 << endl;
        }
    }
    
    cout << "About to instantiate SQlite" << endl;
    FbyDBPtr db(FBYNEW FbySQLiteDB("../var/fby.sqlite3") );
    cout << "About to load" << endl;

    vector<WikiEntryPtr> data;
    db->Load(data, "SELECT * FROM WikiEntry");

    for (int pass = 0; pass < 2; ++pass)
    {
        for (int i = 0; i < data.size(); ++i)
        {
            WikiEntryPtr we(data[i]);
            cout << we->inputname << " and " << we->wikiname << endl;
            cout << "    " << we->needsContentRebuild << " " << we->needsExternalRebuild << endl;

            we->needsContentRebuild = !we->needsContentRebuild;
            we->needsExternalRebuild = !we->needsExternalRebuild;
            db->Write(we);
        }
    }

    vector < string > a;
    db->selectrow_array(a, "SELECT * FROM WikiEntry LIMIT 1");
    cout << "Output from select one wiki entry" << endl;
    for_each(a.begin(), a.end(),
             [](const string &s) { cout << "  " << s << endl; });

    vector <vector < string > > aa;
    db->selectrows_array(aa, "SELECT * FROM WikiEntry");
    if (a.size() == aa[0].size())
    {
        cout << "Got the right number of fields in the first element of selectrows" << endl;
    }

    map < string, string > h;
    db->selectrow_hash(h, "SELECT * FROM WikiEntry LIMIT 1");
    cout << "Output from select one wiki entry" << endl;
    for_each(h.begin(), h.end(),
             [](const pair<string, string> &s) { cout << "  " << s.first << ": " << s.second << endl; });

    vector <map < string, string > > ah;
    db->selectrows_hash(ah, "SELECT * FROM WikiEntry");
    if (h.size() == ah[0].size())
    {
        cout << "Got the right number of fields in the first element of selectrows" << endl;
    }



    return 0;
}
