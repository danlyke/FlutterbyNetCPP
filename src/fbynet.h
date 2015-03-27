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


FBYCLASSPTR(IntervalTimeoutObject);
FBYCLASS(IntervalTimeoutObject) : public ::FbyHelpers::BaseObj
{
    friend class Net;
    FBYUNCOPYABLE(IntervalTimeoutObject);
private:
     std::function<void ()> triggered_function;
     bool stillNeedsEvents;
     bool recurring;
     long microseconds;
     long next_time;
public:
IntervalTimeoutObject(std::function<void()> triggered_function,
                      bool recurring, long microseconds, const char *name, int size)
    :
    BaseObj(name,size),
        triggered_function(triggered_function),
        stillNeedsEvents(true),
        recurring(recurring),
        microseconds(microseconds),
        next_time(microseconds)
    {}
    void unref() { stillNeedsEvents = false; }
    void ref() { stillNeedsEvents = true; }
};

FBYCLASSPTR(TimeoutObject);
FBYCLASS(TimeoutObject) : public IntervalTimeoutObject
{
    friend class Net;

public:
TimeoutObject(std::function<void()> triggered_function,
              long microseconds) : IntervalTimeoutObject(triggered_function,
                                                         false, microseconds, BASEOBJINIT(TimeoutObject))
    {}
};

FBYCLASSPTR(IntervalObject);
FBYCLASS(IntervalObject) : public IntervalTimeoutObject
{
    friend class Net;
public:
IntervalObject(std::function<void()> triggered_function,
               long microseconds) : IntervalTimeoutObject(triggered_function,
                                                          true, microseconds, BASEOBJINIT(IntervalObject))
    {}
};

FBYCLASSPTR(ImmediateObject);
FBYCLASS(ImmediateObject) : public IntervalTimeoutObject
{
    friend class Net;
public:
ImmediateObject(std::function<void()> triggered_function)
    : IntervalTimeoutObject(triggered_function,
                            false, 0, BASEOBJINIT(ImmediateObject))
    {}
};



FBYCLASS(Net) : public ::FbyHelpers::BaseObj
{
public:
    std::vector<ServerPtr> servers;
    std::vector<SocketPtr> sockets;
    std::vector<IntervalTimeoutObjectPtr> timers;

Net() : BaseObj(BASEOBJINIT(Net)), servers(), sockets(), timers() {}
public:
    ServerPtr createServer(CreateServerFunction f);
    void loop();

    TimeoutObjectPtr setTimeout(std::function<void ()> callback,int delay_ms)
    {
        TimeoutObjectPtr timeout(new TimeoutObject(
                                     callback,
                                     delay_ms));
        timers.push_back(timeout);
        return timeout;
    }
    template <typename Arg0> TimeoutObjectPtr setTimeout(std::function<void (Arg0)> callback, int delay_ms,
                                                         Arg0 arg0)
    {
        return setTimeout([=]() { callback(arg0); }, delay_ms);
    }
    void clearTimeout(TimeoutObjectPtr timeout)
    {
        auto timer(std::find(timers.begin(), timers.end(),timeout));
        if (timer != timers.end())
        {
            timers.erase(timer);
        }
    }
    IntervalObjectPtr setInterval(std::function<void ()> callback,int delay_ms)
    {
        IntervalObjectPtr timeout(new IntervalObject(
                                     callback,
                                     delay_ms));
        timers.push_back(timeout);
        return timeout;
    }
    void clearInterval(IntervalObjectPtr interval)
    {
        auto timer(std::find(timers.begin(), timers.end(),interval));
        if (timer != timers.end())
        {
            timers.erase(timer);
        }
    }

    ImmediateObjectPtr setImmediate(std::function<void ()> callback)
    {
        ImmediateObjectPtr timeout(new ImmediateObject(callback));
        timers.push_back(timeout);
        return timeout;
    }
    void clearImmediate(ImmediateObjectPtr immediate)
    {
        auto timer(std::find(timers.begin(), timers.end(), immediate));
        if (timer != timers.end())
        {
            timers.erase(timer);
        }
    }
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


extern bool ServeFile(const char * fileRoot, HTTPRequestPtr request, HTTPResponsePtr response);

#endif /* #ifndef FBYNET_H_INCLUDED */
