#ifndef FBYNET_H_INCLUDED
#define FBYNET_H_INCLUDED

#include "fby.h"


// class Net definition
#include <functional>
#include <map>

FBYCLASSPTR(Socket);
FBYCLASSPTR(Server);
FBYCLASSPTR(Net);

FBYCLASSPTR(HTTPRequest);
FBYCLASSPTR(HTTPResponse);

typedef std::function<void (const char *data, size_t length)> OnDataFunction;
typedef std::function<void ()> OnDrainFunction;
typedef std::function<SocketPtr ()> CreateServerFunction;
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
    
    void SetSocketServerAndFile(Net* net, int fd);
public:
    Socket();
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


FBYCLASS(HTTPResponse) : public Socket
{
public:
    write
}

FBYCLASS(HTTPRequest) : public ::FbyHelpers::BaseObj
{
    friend class HTTPServer;
private: 
    void ConsumeLeadingWhitespace(const char **data, size_t &length);
    void ReadHTTPMethod(const char **data, size_t &length);
    void ConsumeHTTPMethodWhitespace(const char **data, size_t &length);
    void ReadHTTPPath(const char **data, size_t &length);
    void ConsumeHTTPPathWhitespace(const char **data, size_t &length);
    void ReadHTTPProtocol(const char **data, size_t &length);
    void ConsumeHTTPProtocolNewline(const char **data, size_t &length);
    void ReadHTTPHeaderName(const char **data, size_t &length);
    void ReadHTTPHeaderNameContinue(const char **data, size_t &length);
    void ConsumeHTTPHeaderNameWhitespace(const char **data, size_t &length);
    void ReadHTTPHeaderValue(const char **data, size_t &length);
    void ReadHTTPRequestData(const char **data, size_t &length);


    void ResetReadState();

protected:
    virtual void EmitNameValue(std::string name, const std::string &value);

public:
    void (HTTPRequest::*readState)(const char **data, size_t &length);

    std::string method;
    std::string path;
    std::string protocol;
    std::map<std::string, std::string> headers;

    std::string headerName;
    std::string headerValue;
    void ReadData(const char *data, size_t length);
    HTTPRequest();
};


FBYCLASS(HTTPServer) : public Server
{
public:
                     HTTPServer(Net *net, RespondToHTTPRequestFunction f);
};


inline HTTPServer::HTTPServer(Net *net, RespondToHTTPRequestFunction)
                  : Server(net, []() {return SocketPtr(new Socket);})
{
}


inline Socket::Socket()
: BaseObj(BASEOBJINIT(Socket)),
              fd(-1), 
              on_data(), 
              on_drain(),
              net(),
              queuedWrite(), emitDrain(false)
{
}

inline void Socket::SetSocketServerAndFile(Net* net, int fd)
{
    std::cout << "Setting socket server and file " << fd << std::endl;
    this->net = net;
    this->fd = fd;
}


inline Server::Server(Net * net, CreateServerFunction create_func)
: BaseObj(BASEOBJINIT(Server)),
    create_func(create_func), fd(-1), net(net)
{
}


#endif /* #ifndef FBYNET_H_INCLUDED */
