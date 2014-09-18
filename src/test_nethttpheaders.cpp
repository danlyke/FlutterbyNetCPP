#include "fby_net.h"

const char *requestText =
    "GET / HTTP/1.1\r\n"
    "Host: home.flutterby.net\r\n"
    "\r\n";

int main(int, char **)
{
    HTTPRequestPtr request(new HTTPRequest);
    return 0;
}
