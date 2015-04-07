#include "fby.h"
#include "fbynet.h"
#include "fbyregex.h"
#include <gtest/gtest.h>

#include <string>
using namespace std;
using namespace FbyHelpers;


const char testbody[] =
    "start_time_1=5%3A00&start_time_2=20%3A00&name_0=Valve+1&time"
    "_0=1";

#if 0
    "&valve_Mon_am_0=on&valve_Mon_pm_0=on&valve_Tue_am_0=on&v"
    "alve_Tue_pm_0=on&valve_Wed_am_0=on&valve_Wed_pm_0=on&valve_T"
    "hu_am_0=on&valve_Thu_pm_0=on&valve_Fri_am_0=on&valve_Fri_pm_"
    "0=on&valve_Sat_am_0=on&valve_Sat_pm_0=on&valve_Sun_am_0=on&v"
    "alve_Sun_pm_0=on&name_1=Valve+2&time_1=2&valve_Mon_am_1=on&v"
    "alve_Mon_pm_1=on&valve_Tue_am_1=on&valve_Tue_pm_1=on&valve_W"
    "ed_am_1=on&valve_Wed_pm_1=on&valve_Thu_am_1=on&valve_Thu_pm_"
    "1=on&valve_Fri_am_1=on&valve_Fri_pm_1=on&valve_Sat_am_1=on&v"
    "alve_Sat_pm_1=on&valve_Sun_am_1=on&valve_Sun_pm_1=on&name_2="
    "Valve+3&time_2=3&valve_Mon_am_2=on&valve_Mon_pm_2=on&valve_T"
    "ue_am_2=on&valve_Tue_pm_2=on&valve_Wed_am_2=on&valve_Wed_pm_"
    "2=on&valve_Thu_am_2=on&valve_Thu_pm_2=on&valve_Fri_am_2=on&v"
    "alve_Fri_pm_2=on&valve_Sat_am_2=on&valve_Sat_pm_2=on&valve_S"
    "un_am_2=on&valve_Sun_pm_2=on&name_3=Valve+4&time_3=4&valve_M"
    "on_am_3=on&valve_Mon_pm_3=on&valve_Tue_am_3=on&valve_Tue_pm_"
    "3=on&valve_Wed_am_3=on&valve_Wed_pm_3=on&valve_Thu_am_3=on&v"
    "alve_Thu_pm_3=on&valve_Fri_am_3=on&valve_Fri_pm_3=on&valve_S"
    "at_am_3=on&valve_Sat_pm_3=on&valve_Sun_am_3=on&valve_Sun_pm_"
    "3=on&name_4=Valve+5&time_4=5&valve_Mon_am_4=on&valve_Mon_pm_"
    "4=on&valve_Tue_am_4=on&valve_Tue_pm_4=on&valve_Wed_am_4=on&v"
    "alve_Wed_pm_4=on&valve_Thu_am_4=on&valve_Thu_pm_4=on&valve_F"
    "ri_am_4=on&valve_Fri_pm_4=on&valve_Sat_am_4=on&valve_Sat_pm_"
    "4=on&valve_Sun_am_4=on&valve_Sun_pm_4=on&name_5=Valve+6&time"
    "_5=6&valve_Mon_am_5=on&valve_Mon_pm_5=on&valve_Tue_am_5=on&v"
    "alve_Tue_pm_5=on&valve_Wed_am_5=on&valve_Wed_pm_5=on&valve_T"
    "hu_am_5=on&valve_Thu_pm_5=on&valve_Fri_am_5=on&valve_Fri_pm_"
    "5=on&valve_Sat_am_5=on&valve_Sat_pm_5=on&valve_Sun_am_5=on&v"
    "alve_Sun_pm_5=on&name_6=Valve+7&time_6=7&valve_Mon_am_6=on&v"
    "alve_Mon_pm_6=on&valve_Tue_am_6=on&valve_Tue_pm_6=on&valve_W"
    "ed_am_6=on&valve_Wed_pm_6=on&valve_Thu_am_6=on&valve_Thu_pm_"
    "6=on&valve_Fri_am_6=on&valve_Fri_pm_6=on&valve_Sat_am_6=on&v"
    "alve_Sat_pm_6=on&valve_Sun_am_6=on&valve_Sun_pm_6=on&name_7="
    "Valve+8&time_7=8&valve_Mon_am_7=on&valve_Mon_pm_7=on&valve_T"
    "ue_am_7=on&valve_Tue_pm_7=on&valve_Wed_am_7=on&valve_Wed_pm_"
    "7=on&valve_Thu_am_7=on&valve_Thu_pm_7=on&valve_Fri_am_7=on&v"
    "alve_Fri_pm_7=on&valve_Sat_am_7=on&valve_Sat_pm_7=on&valve_S"
    "un_am_7=on&valve_Sun_pm_7=on&Save=Save";
#endif


//TEST(HeaderTest,ParseBody)
int main(int, char **)
{

    BodyParserURLEncodedPtr parseptr(new BodyParserURLEncoded);
    map<string,string> namevaluepairs;

    parseptr->onNameValue(
        [&namevaluepairs](const std::string &name, const std::string &value)
        {
            namevaluepairs[name] = value;
            cout << "Found " << name << ": '" << value << "'" << endl;
        });
    size_t blocksize = 1;
    size_t start = 0;
    size_t length = sizeof(testbody) - 1;

    while (length)
    {
        if (blocksize > length)
            blocksize = length;
        parseptr->on_data(testbody + start, blocksize);
        start += blocksize;
        length -= blocksize;
        ++blocksize;
    }
    parseptr->on_end();

//     EXPECT_EQ(namevaluepairs["start_time_1"], string("5:00"));
//     EXPECT_EQ(namevaluepairs["start_time_2"], string("20:00"));
//     EXPECT_EQ(namevaluepairs["name_0"], string("Valve+1"));
//     EXPECT_EQ(namevaluepairs["time_0"], string("1"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_am_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_pm_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_am_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_pm_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_am_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_pm_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_am_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_pm_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_am_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_pm_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_am_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_pm_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_am_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_pm_0"], string("on"));
//     EXPECT_EQ(namevaluepairs["name_1"], string("Valve+2"));
//     EXPECT_EQ(namevaluepairs["time_1"], string("2"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_am_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_pm_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_am_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_pm_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_am_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_pm_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_am_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_pm_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_am_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_pm_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_am_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_pm_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_am_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_pm_1"], string("on"));
//     EXPECT_EQ(namevaluepairs["name_2"], string("Valve+3"));
//     EXPECT_EQ(namevaluepairs["time_2"], string("3"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_am_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_pm_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_am_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_pm_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_am_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_pm_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_am_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_pm_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_am_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_pm_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_am_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_pm_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_am_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_pm_2"], string("on"));
//     EXPECT_EQ(namevaluepairs["name_3"], string("Valve+4"));
//     EXPECT_EQ(namevaluepairs["time_3"], string("4"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_am_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_pm_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_am_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_pm_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_am_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_pm_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_am_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_pm_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_am_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_pm_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_am_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_pm_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_am_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_pm_3"], string("on"));
//     EXPECT_EQ(namevaluepairs["name_4"], string("Valve+5"));
//     EXPECT_EQ(namevaluepairs["time_4"], string("5"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_am_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_pm_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_am_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_pm_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_am_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_pm_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_am_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_pm_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_am_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_pm_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_am_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_pm_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_am_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_pm_4"], string("on"));
//     EXPECT_EQ(namevaluepairs["name_5"], string("Valve+6"));
//     EXPECT_EQ(namevaluepairs["time_5"], string("6"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_am_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_pm_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_am_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_pm_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_am_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_pm_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_am_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_pm_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_am_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_pm_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_am_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_pm_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_am_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_pm_5"], string("on"));
//     EXPECT_EQ(namevaluepairs["name_6"], string("Valve+7"));
//     EXPECT_EQ(namevaluepairs["time_6"], string("7"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_am_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_pm_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_am_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_pm_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_am_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_pm_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_am_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_pm_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_am_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_pm_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_am_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_pm_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_am_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_pm_6"], string("on"));
//     EXPECT_EQ(namevaluepairs["name_7"], string("Valve+8"));
//     EXPECT_EQ(namevaluepairs["time_7"], string("8"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_am_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Mon_pm_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_am_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Tue_pm_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_am_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Wed_pm_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_am_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Thu_pm_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_am_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Fri_pm_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_am_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sat_pm_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_am_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["valve_Sun_pm_7"], string("on"));
//     EXPECT_EQ(namevaluepairs["Save"], string("Save"));
}
