#include "irrigation.h"

using namespace std;

string startTime1("5:00");
string startTime2("19:00");


Valve::Valve()
 : 
    BaseObj(BASEOBJINIT(Valve)),
    name(), valve_num(), run_time(), remaining_run_time(), days(),timer()
{
}

Valve::Valve(int valve_num)
 : 
    BaseObj(BASEOBJINIT(Valve)),
    name("Valve "), valve_num(valve_num), run_time(0), remaining_run_time(0), days(0),timer()
{
    char ch[2];
    if (valve_num >= 0 && valve_num <= 9)
    {
        ch[0] = '0' + valve_num;
        ch[1] = '\0';
        name += ch;
    }
}

void Valve::TurnOff()
{
    remaining_run_time = 0;
    cout << "Turning off valve " << valve_num << endl;
}
void Valve::TurnOn()
{
    remaining_run_time = run_time * 60;
    cout << "Turning on valve " << valve_num << endl;
}
