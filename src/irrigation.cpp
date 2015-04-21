#include "irrigation.h"
#include <ftdi.h>





using namespace std;


struct ftdi_context *ftdi(NULL);
bool StartFTDI()
{
    int ret;
    if ((ftdi = ftdi_new()) == 0)
    {
        // fprintf(stderr, "ftdi_new failed\n");
        return false;
    }
//    version = ftdi_get_library_version();
//    printf("Initialized libftdi %s (major: %d, minor: %d, micro: %d, snapshot ver: %s)\n",
//           version.version_str, version.major, version.minor, version.micro,
//           version.snapshot_str);
    if ((ret = ftdi_usb_open(ftdi, 0x0403, 0x6001)) < 0)
    {
        // fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        ftdi_free(ftdi);
        return false;
    }
// Read out FTDIChip-ID of R type chips
    if (ftdi->type == TYPE_R)
    {
        unsigned int chipid;
        printf("ftdi_read_chipid: %d\n", ftdi_read_chipid(ftdi, &chipid));
        printf("FTDI chipid: %X\n", chipid);
    }
    ftdi_set_bitmode(ftdi, 0xFF, BITMODE_BITBANG);
    return true;
}


void SetFTDI(int bit)
{
    unsigned char buf[1] = {0x0};
    if (bit == 0)
    {
        printf("Turning off relays (%x)\n", buf[0]);
    }
    else
    {
        --bit;
        buf[0] ^= (1 << bit);
        printf("Turning on relay %d (%x)\n", bit, buf[0]);
    }

    if (ftdi)
        ftdi_write_data(ftdi, buf, 1);
}


void StopFTDI()
{
    SetFTDI(0);
    int ret;
    if (ftdi)
    {
        if ((ret = ftdi_usb_close(ftdi)) < 0)
        {
            // fprintf(stderr, "unable to close ftdi device: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
        }
        ftdi_free(ftdi);
        ftdi = NULL;
    }
}


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

void Valve::TurnOff(NetPtr net)
{
    net->clearInterval(timer);
    timer = IntervalObjectPtr();
    remaining_run_time = 0;
    SetFTDI(0);
    //cout << "Turning off valve " << valve_num << endl;
}

void Valve::TurnOn(NetPtr net)
{
    net->clearInterval(timer);
    remaining_run_time = run_time * 60;
    ValvePtr valve(this);
    timer = net->setInterval([valve, net]()
                             {
                                 --valve->remaining_run_time;
                                 if (valve->remaining_run_time <= 0)
                                 {
                                     valve->TurnOff(net);
                                 }
                             }, 1000);
    //cout << "Turning on valve " << valve_num << endl;
    SetFTDI(valve_num + 1);
}
