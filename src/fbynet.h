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
typedef std::function<void (SocketPtr)> CreateServerFunction;
typedef std::function<void (HTTPRequestPtr request, HTTPResponsePtr response)> RespondToHTTPRequestFunction;
typedef std::function<HTTPResponsePtr (std::string host, std::string method, std::string path,
                                       std::vector< std::string, std::string> attributes ) > CreateHTTPResponseFunction;

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
    bool doneWithWrites;
    
    void SetSocketServerAndFile(Net* net, int fd);
public:
    Socket();
    void onData(OnDataFunction on_data)
    {
        this->on_data = on_data;
    }

    void onDrain(OnDrainFunction on_drain)
    {
        this->on_drain = on_drain;
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

    bool end(const char *data, size_t length)
    {
        std::cout << "Writing end " << (unsigned long)(this) << std::endl;
        doneWithWrites = true;
        return write(data, length);
    }
    bool end(const std::string &s)
    {
        std::cout << "Writing end " << (unsigned long)(this) << std::endl;
        doneWithWrites = true;
        return write(s);
    }
    bool end(const char *data)
    {
        std::cout << "Writing end " << (unsigned long)(this) << std::endl;
        doneWithWrites = true;
        return write(data);
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


FBYCLASS(HTTPResponse) : public ::FbyHelpers::BaseObj
{
    FBYUNCOPYABLE(HTTPResponse);
private:
    SocketPtr socket;
    bool wroteHeader;

public:
    HTTPResponse(SocketPtr);
    void writeHead( int resultCode,
               const std::map<std::string,std::string> &);
    void writeHead( int resultCode);
    void writeHead( int resultCode, const char *);
    void redirect(const std::string &path);
    void respondHTML(int code, std::map<std::string, std::string> attrs,
                     const std::string &title, const std::string &body);
    void respondHTML(int code, 
                     const std::string &title, const std::string &body);
    void respondHTML(int code, const std::string &body);

    bool write(const char *data, size_t length) { return socket->write(data,length);}
    bool write(const std::string &s) { return socket->write(s);}
    bool write(const char *data) { return socket->write(data);}
    bool end(const char *);
    bool end(const char *data, size_t length);
    bool end(const std::string &s);
    void onDrain(OnDrainFunction on_drain)
    {
        socket->onDrain(on_drain);
    }
};

FBYCLASS(HTTPRoute) : public ::FbyHelpers::BaseObj
{
public:

    bool wants(const std::string &host, const std::string &method, const std::string &path);
};


FBYCLASSPTR(HTTPRequestBuilder);
FBYCLASSPTR(HTTPRequest);
FBYCLASS(HTTPRequest) : public ::FbyHelpers::BaseObj
{
    friend class HTTPRequestBuilder;
public:
    std::string method;
    std::string path;
    std::string protocol;
    std::map<std::string, std::string> headers;

    HTTPRequest();
};


FBYCLASS(HTTPRequestBuilder) : public ::FbyHelpers::BaseObj
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
    void ReadHTTPRequestBuilderData(const char **data, size_t &length);

    void ResetReadState();

    void GenerateHTTPResponder();
protected:
    virtual void EmitNameValue(std::string name, const std::string &value);
   
    RespondToHTTPRequestFunction on_request;
    SocketPtr socket;
public:
    void (HTTPRequestBuilder::*readState)(const char **data, size_t &length);

    HTTPRequestPtr request;

    std::string headerName;
    std::string headerValue;
    void ReadData(const char *data, size_t length);
    HTTPRequestBuilder(SocketPtr socket, RespondToHTTPRequestFunction);
};

FBYCLASSPTR(HTTPServer);

FBYCLASS(HTTPServer) : public Server
{
public:
    HTTPServer(Net *net, RespondToHTTPRequestFunction f);
    
};


//inline HTTPServer::HTTPServer(Net *net, RespondToHTTPRequestFunction)
//                  : Server(net, []() {})
//{
//}


inline Socket::Socket()
: BaseObj(BASEOBJINIT(Socket)),
              fd(-1), 
              on_data(), 
              on_drain(),
              net(),
              queuedWrite(), emitDrain(false),
              doneWithWrites(false)
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


extern bool ServeFile(HTTPRequestPtr request, HTTPResponsePtr response);

#endif /* #ifndef FBYNET_H_INCLUDED */
