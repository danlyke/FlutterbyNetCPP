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

void Valve::TurnOff()
{
    cout << "Turning off valve " << valve_num << endl;
}
void Valve::TurnOn()
{
    remaining_run_time = run_time * 60;
    cout << "Turning on valve " << valve_num << endl;
}
