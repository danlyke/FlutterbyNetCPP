#include "fbynet.h"
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;



bool ServeFile(HTTPRequestPtr request, HTTPResponsePtr response)
{
    int fd;
    string path("../t/html");
    path += request->path;
    if (0 < (fd = open(path.c_str(), O_RDONLY)))
    {
        response->writeHead(200);
        int buflen = 8192;
        char *buffer = new char[buflen];
        int len = read(fd, buffer, buflen);
        if (len < buflen)
        {
            response->end(buffer, len);
            close(fd);
            delete[] buffer;
        }
        else
        {
            response->write(buffer, len);
            response->onDrain([buflen, buffer, fd, response]()
                              {
                                  int len = read(fd, buffer, buflen);
                                  if (len < buflen)
                                  {
                                      response->end(buffer, len);
                                      close(fd);
                                      delete[] buffer;
                                  }
                                  else
                                  {
                                      response->write(buffer, len);
                                  }
                              });
        }

        return true;
    }
    return false;
}


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
                                      for (auto v = request->headers.begin();
                                           v != request->headers.end();
                                           ++v)
                                      {
                                          cout << "   " << v->first << " : " << v->second << endl;
                                      }
                                      if (!ServeFile(request,response))
                                      {
                                          response->writeHead(404);
                                          response->end("<html><head><title>Nope!</title></head><body><h1>Ain't there, dude!</h1></body></html>\n");
                                      }
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
