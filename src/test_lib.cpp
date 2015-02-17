#include <gtest/gtest.h>
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

TEST(TimesConvertToThemself,TimesConvertBackToThemselves)
{
    for (int i = 0; testDates[i]; ++i)
    {
        time_t t(TextDateToTime(testDates[i]));
        string ts(TimeToTextDate(t));
        EXPECT_EQ(ts,string(testDates[i]));
    }
}
