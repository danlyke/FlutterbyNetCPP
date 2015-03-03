#include "fbynet.h"
#include <string>
using namespace std;

int main(int argc, char **argv)
{
    Net *net = new Net();

    net->createServer([](SocketPtr socket)
                     {
                         HTTPRequestBuilderPtr requestBuilderPtr(new HTTPRequestBuilder);
                         HTTPResponsePtr requestBuilderPtr(new HTTPResponse(socket));
                         
                         socket->onData(
                             [&socket &requestBuilderPtr](const char *data, int length)
                             {
                                 requestBuilder->ReadData(data, length);
                             });
                     })->listen(5000);
    net->loop();
}
