#include "fbynet.h"
#include <string>
using namespace std;

int main(int argc, char **argv)
{
    Net *net = new Net();

    net->createServer([](SocketPtr socket)
                     {
                         HTTPRequestBuilderPtr requestBuilder
                             (new HTTPRequestBuilder
                              (socket,
                                  [](HTTPRequestPtr request, HTTPResponsePtr response)
                                  {
                                      cout << "Request: '" << request->method
                                           << "' '" << request->path
                                           << "' '" << request->protocol
                                           << "'" << endl;
                                      for (auto v = request->headers.begin();
                                           v != request->headers.end();
                                           ++v)
                                      {
                                          cout << "   " << v->first << " : " << v->second << endl;
                                      }
                                      response->writeHead(200);
                                      response->end("<html><head><title>Test</title></head><body><h1>Test</h1></body></html>\n");
                                  }
                                  ));
                         
                         socket->onData(
                             [socket, requestBuilder](const char *data, int length)
                             {
                                 requestBuilder->ReadData(data, length);
                             });
                     })->listen(5000);
    net->loop();
}
