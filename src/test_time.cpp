#include "fby.h"
#include "fbyregex.h"
#include <ostream>
#include <time.h>

using namespace std;


int main(int argc, char **argv)
{
    Regex regexTime(R"((\d+)\:(\d+))");
    string five("5:00");
    time_t now = time(NULL);
    struct tm local_time;
    localtime_r(&now, &local_time);




    cout << "int tm_sec " << local_time.tm_sec << endl;
    cout << "int tm_min " << local_time.tm_min << endl;
    cout << "int tm_hour " << local_time.tm_hour << endl;
    cout << "int tm_mday " << local_time.tm_mday << endl;
    cout << "int tm_mon " << local_time.tm_mon << endl;
    cout << "int tm_year " << local_time.tm_year << endl;
    cout << "int tm_wday " << local_time.tm_wday << endl;
    cout << "int tm_yday " << local_time.tm_yday << endl;
    cout << "int tm_isdst " << local_time.tm_isdst << endl;
    cout << "long int tm_gmtoff " << local_time.tm_gmtoff << endl;
    cout << "const char *tm_zone " << local_time.tm_zone << endl;

    RegexMatch match;

    if (regexTime.Match(five, match))
    {
        cout << "Timezone " << timezone << endl;
        cout << "Hours " << match.Match(1) << endl;
        cout << "Minutes " << match.Match(2) << endl;

    }
    return 0;
}
