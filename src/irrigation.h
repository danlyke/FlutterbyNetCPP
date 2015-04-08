#ifndef IRRIGATION_H_INCLUDED
#define IRRIGATION_H_INCLUDED
#include "fby.h"
#include "fbynet.h"

extern std::string startTime1;
extern std::string startTime2;
FBYCLASSPTR(Valve);
FBYCLASS(Valve) : public FbyHelpers::BaseObj {
public:
    std::string name;
    int valve_num;
    int run_time;
    int remaining_run_time;
    int days;
    IntervalObjectPtr timer;
    

    Valve();
    Valve(int valve_num);
    void TurnOff();
    void TurnOn();
};

#endif /* #ifndef IRRIGATION_H_INCLUDED */
