#include "fby.h"
#include "fbynet.h"
#include "fbyregex.h"

#include <string>
using namespace std;


const char testbody[] =
    "start_time_1=5%3A00&start_time_2=20%3A00&name_0=Valve+1&time"
    "_0=1&valve_Mon_am_0=on&valve_Mon_pm_0=on&valve_Tue_am_0=on&v"
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



int main(int argc, char **argv)
{
    BodyParserURLEncodedPtr parseptr(new BodyParserURLEncoded);
    parse.onNameValue(
        [](const std::string &name, const std::string &value)
        {
            cout << "Got " << name << ": '" << value << "'" << endl;
        });
    parse.on_data(testbody, sizeof(testbody));
    parse.on_end();
    return 0;

}
