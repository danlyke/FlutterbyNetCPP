#ifndef FBYNET_H_INCLUDED
#define FBYNET_H_INCLUDED

#include "fby.h"


// class Net definition
#include <functional>

FBYCLASSPTR(Socket);
FBYCLASSPTR(Server);
FBYCLASSPTR(Net);

typedef std::function<void (const char *data, size_t length)> OnDataFunction;
typedef std::function<void (SocketPtr socket)> CreateServerFunction;

FBYCLASS(Socket) : public ::FbyHelpers::BaseObj
{
    friend class Net;
    
private:
    Socket(const Socket &);
    Socket &operator=(const Socket&);
    
    int fd;
    OnDataFunction on_data;
    Net * net;


public:
    Socket(NetPtr net, int fd);
    void onData(OnDataFunction on_data)
    {
        this->on_data = on_data;
    }


    void write(const char *data, size_t length);
    void write(const std::string &s)
    {
        write(s.data(), s.length());
    }
    void write(const char *data)
    {
        write(data, strlen(data));
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
    ServerPtr createServer(CreateServerFunction f)
    {
        return ServerPtr(new Server(this, f));
    };

    void loop();
};





inline Socket::Socket(NetPtr net, int fd)
: BaseObj(BASEOBJINIT(Socket)),
   fd(fd), on_data(), net()   
{
}


inline Server::Server(Net * net, CreateServerFunction create_func)
: BaseObj(BASEOBJINIT(Server)),
    create_func(create_func), fd(-1), net(net)
{
}


#endif /* #ifndef FBYNET_H_INCLUDED */
