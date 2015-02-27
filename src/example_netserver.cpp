#include "fbynet.h"
#include <string>
using namespace std;

int main(int argc, char **argv)
{
    Net *net = new Net();

    net->createServer([]()
                      {
                          SocketPtr socket(new Socket());
                          socket->onData(
                              [socket](const char *data, int length)
                              {
                                  string s("Got ");
                                  s += string(data, length);
                                  s += "\n";
                                  cout << s;
                                  cout << "Writing" << endl;
                                  socket->write(s);
                                  cout << "Wrote" << endl;
                              });
                          return socket;
                      })->listen(5000);
    net->createServer([]()
                      {
                          HTTPServerPtr server(new HTTPServer());
                          HTTPRoutePtr 
                          socket->onData(
                              [socket](const char *data, int length)
                              {
                                  string s("Got ");
                                  s += string(data, length);
                                  s += "\n";
                                  cout << s;
                                  cout << "Writing" << endl;
                                  socket->write(s);
                                  cout << "Wrote" << endl;
                              });
                          return socket;
                      })->listen(8888);
    net->loop();
}
