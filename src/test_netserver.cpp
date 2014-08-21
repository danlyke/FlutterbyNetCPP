#include "fbynet.h"
#include <string>
using namespace std;

int main(int argc, char **argv)
{
    Net *net = new Net();

    net->createServer([](SocketPtr socket)
                     {
                         fprintf(stderr, "Create lambda: %lx\n", (unsigned long)(&*socket));
                         socket->onData(
                             [socket](const char *data, int length)
                             {
                                 fprintf(stderr, "Data lambda: %lx\n", (unsigned long)(&*socket));
                                 string s("Got ");
                                 s += string(data, length);
                                 s += "\n";
                                 cout << s;
                                 socket->write(s);
                             });
                     })->listen(5000);
    net->loop();
}
