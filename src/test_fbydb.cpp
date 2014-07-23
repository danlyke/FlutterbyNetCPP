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

#if 1
    DYNARRAY(WikiEntryPtr) data;
    db->Load(data, "SELECT * FROM WikiEntry");
#elif 0
    DYNARRAY(FbyORMPtr) data = db->Load([](){ return FbyORMPtr(FBYNEW WikiEntry()); }, "SELECT * FROM WikiEntry");
#else
    DYNARRAY(WikiEntryPtr) data;
    db->CastCopyArray(db->Load([](){ return FbyORMPtr(FBYNEW WikiEntry()); }, "SELECT * FROM WikiEntry"), data);
#endif

    for (int pass = 0; pass < 2; ++pass)
    {
        for (int i = 0; i < data->Count; ++i)
        {
            FbyORMPtr obj = data[i];
            WikiEntryPtr we(dynamic_cast<WikiEntry *>(&*obj));
            cout << we->inputname << " and " << we->wikiname << endl;
            cout << "    " << we->needsContentRebuild << " " << we->needsExternalRebuild << endl;

            we->needsContentRebuild = !we->needsContentRebuild;
            we->needsExternalRebuild = !we->needsExternalRebuild;
            db->Write(we);
        }
    }

    
    return 0;
}
