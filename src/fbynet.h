#ifndef FBYNET_H_INCLUDED
#define FBYNET_H_INCLUDED

#include "fby.h"


// class Net definition
#include <functional>

FBYCLASSPTR(Socket);
FBYCLASSPTR(Server);
FBYCLASSPTR(Net);

FBYCLASSPTR(HTTPRequest);
FBYCLASSPTR(HTTPResponse);

typedef std::function<void (const char *data, size_t length)> OnDataFunction;
typedef std::function<void ()> OnDrainFunction;
typedef std::function<void (SocketPtr socket)> CreateServerFunction;
typedef std::function<void (HTTPRequestPtr request, HTTPResponsePtr response)> RespondToHTTPRequestFunction;

FBYCLASS(Socket) : public ::FbyHelpers::BaseObj
{
    friend class Net;
    
private:
    Socket(const Socket &);
    Socket &operator=(const Socket&);
    
    int fd;
    OnDataFunction on_data;
    OnDrainFunction on_drain;
    Net * net;
    std::string queuedWrite;
    bool emitDrain;
public:
    Socket(Net* net, int fd);
    void onData(OnDataFunction on_data)
    {
        this->on_data = on_data;
    }


    bool write(const char *data, size_t length);
    bool write(const std::string &s)
    {
        return write(s.data(), s.length());
    }
    bool write(const char *data)
    {
        return write(data, strlen(data));
    }
};



FBYCLASS(Server) : public ::FbyHelpers::BaseObj
{
    friend class Net;
private:
    Server(const Server &);
    Server &operator=(const Server &);

    CreateServerFunction create_func;
    int fd;
    Net *net;

public:
    ServerPtr listen(int socket);
    Server(Net * net, CreateServerFunction create_func);
};


FBYCLASS(Net) : public ::FbyHelpers::BaseObj
{
public:
    std::vector<ServerPtr> servers;
    std::vector<SocketPtr> sockets;

Net() : BaseObj(BASEOBJINIT(Net)), servers(), sockets() {}
public:
    ServerPtr createServer(CreateServerFunction f);
    void loop();
};



FBYCCLASS(HTTPRequest) : public ::FbyHelpers::BaseObj
{
public:
    int readState;
    string method;
    string path;
    string protocol;
    std::map< std::string, std::string > headers;
    void ReadData(const char *data, size_t length);
};


FBYCLASS(HTTPServer) : public Server
{
public:
    HTTPServer(Net *net) {
    }
};


inline HTTPServer::HTTPServer(Net *net, RespondToHTTPRequestFunction f)
                  : Server(net), 
{
}


inline Socket::Socket(Net* net, int fd)
: BaseObj(BASEOBJINIT(Socket)),
              fd(fd), 
              on_data(), 
              on_drain(),
              net(net),
              queuedWrite(), emitDrain(false)
{
}


inline Server::Server(Net * net, CreateServerFunction create_func)
: BaseObj(BASEOBJINIT(Server)),
    create_func(create_func), fd(-1), net(net)
{
}


#endif /* #ifndef FBYNET_H_INCLUDED */
