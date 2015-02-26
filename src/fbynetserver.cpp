#include "fbynet.h"


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <ctype.h>

#include "fbynet.h"
#include <iostream>

using namespace FbyHelpers;

int termsig = 0;
int got_sighup = 0;
int listensocket;
#define KEEPALIVE_TIME 20

static void catchsignal_TERMINT(int sig)
{
	termsig = sig;
}

static void catchsignal_HUP(int)
{
	got_sighup = 1;
}


struct SignalCatch
{
    int sig;
    void (*handler)(int signal);
} signal_handlers[] =
{
    { SIGPIPE, SIG_IGN },
    { SIGCHLD, SIG_IGN },
    { SIGHUP, catchsignal_HUP },
    { SIGTERM, catchsignal_TERMINT },
};

struct {
    struct sigaction new_signal_action;
    struct sigaction old_signal_action;
}  signal_actions[sizeof(signal_handlers)/sizeof(*signal_handlers)];


void catchsignals()
{
    for (size_t i = 0; i < sizeof(signal_handlers) / sizeof(signal_handlers[0]); ++i)
    {
        memset(&signal_actions[i].new_signal_action,0,
               sizeof(signal_actions[i].new_signal_action));
        sigemptyset(&signal_actions[i].new_signal_action.sa_mask);
        signal_actions[i].new_signal_action.sa_handler = 
            signal_handlers[i].handler;
        sigaction(signal_handlers[i].sig, &signal_actions[i].new_signal_action, 
                  &signal_actions[i].old_signal_action);
    }
}

void releasesignals()
{
    for (size_t i = 0; 
         i < sizeof(signal_handlers) / sizeof(signal_handlers[0]); ++i)
    {
        sigaction(signal_handlers[i].sig, &signal_actions[i].old_signal_action, 
                  &signal_actions[i].new_signal_action);
    }
}

ServerPtr Server::listen(int socket_num)
{
    int socket_opt;
    int resultCode;
    struct addrinfo          ask,*res = NULL;
    memset(&ask, 0, sizeof(ask));
    ask.ai_flags = AI_PASSIVE;
    ask.ai_socktype = SOCK_STREAM;
    char socket_ch[16];
    snprintf(socket_ch, sizeof(socket_ch), "%d", socket_num);
    
    if (0 != (resultCode = getaddrinfo(NULL, socket_ch, &ask, &res))) 
    {
        fprintf(stderr,"getaddrinfo (ipv4): %s\n",gai_strerror(resultCode));
        exit(1);
    }

    fprintf(stderr, "About to create socket for %d %d %d\n",res->ai_family, res->ai_socktype, res->ai_protocol);
    fprintf(stderr, "Compare to %d %d %d\n",AF_INET, SOCK_STREAM, 0);

    fd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    socket_opt = 1; 
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&socket_opt,sizeof(socket_opt));
    fcntl(fd,F_SETFL,O_NONBLOCK);
    struct sockaddr_storage  ss;
    memcpy(&ss,res->ai_addr,res->ai_addrlen);
    
    if (-1 == bind(fd, (struct sockaddr*)&ss, res->ai_addrlen)) 
    {
        perror("bind");
        exit(1);
    }
    
    if (-1 == ::listen(fd, 8)) 
    {
        perror("listen");
        exit(1);
    }
    
    return ServerPtr(this);
}

bool Socket::write(char const *data, size_t size)
{
    fprintf(stderr, "Writing %*s to %d\n", (int)size, data, fd);
    bool wroteEverything(false);
    ssize_t bytes_written = ::write(fd, data, size);

    fprintf(stderr, "Wrote %d\n", (int)bytes_written);
    if (bytes_written > 0)
    { 
        if (bytes_written < (ssize_t)size)
        {
            queuedWrite = std::string(data+bytes_written, size - bytes_written);
        }
        else
        {
            wroteEverything  = true;
            emitDrain = true;
        }
    }
    else
    {
        perror("Error writing");
    }
    return wroteEverything;
}

ServerPtr Net::createServer(CreateServerFunction f)
{
    ServerPtr server(new Server(this, f));
    servers.push_back(server);
    return server;
};


void
Net::loop()
{
	time_t now;
	struct timeval      tv;
	int                 max_fd;
	fd_set              read_fds,write_fds;

    fprintf(stderr, "Beginning of loop\n");
    fprintf(stderr, "Server size is %d sockets size is %d\n",
            (int)(servers.size()), (int)(sockets.size()));
    termsig = 0;
    catchsignals();

	for (;(servers.size() || sockets.size()) && !termsig;) 
	{
        fprintf(stderr, "Beginning of event loop\n");
		FD_ZERO(&read_fds);
		FD_ZERO(&write_fds);

		max_fd = 0;
		/* add listening socket */
        for (auto server = servers.begin(); server != servers.end(); ++server)
        {
            FD_SET((*server)->fd,&read_fds);
            if ((*server)->fd > max_fd)
                max_fd = (*server)->fd;
		}
        for (auto socket = sockets.begin(); socket != sockets.end(); ++socket)
        {
            FD_SET((*socket)->fd, &read_fds);
            if ((*socket)->fd > max_fd)
                max_fd = (*socket)->fd;
            if ((*socket)->queuedWrite.size())
                FD_SET((*socket)->fd, &write_fds);
		}

        /* CHECK FOR WRITES HERE! */


		/* go! */
		tv.tv_sec  = KEEPALIVE_TIME;
		tv.tv_usec = 0;
		if (-1 == select(max_fd+1,
				 &read_fds,
				 &write_fds,
				 NULL,&tv))
		{
			continue;
		}
		now = time(NULL);

        for (auto server = servers.begin(); server != servers.end(); ++server)
        {
            if (FD_ISSET((*server)->fd,&read_fds)) 
            {
                int fd = accept((*server)->fd, NULL, NULL);
				if (-1 == fd)
				{
					if (EAGAIN != errno)
						perror("accept");
				}	
				else
				{
                    SocketPtr socket(new Socket(this,fd));
					fcntl(fd,F_SETFL,O_NONBLOCK);
                    (*server)->create_func(socket);
                    sockets.push_back(socket);
					fprintf(stderr,"%03d: new request (%d)\n",
                            (*server)->fd, fd);
				}
			}
		}
		
        for (auto socket = sockets.begin(); socket != sockets.end(); ++socket)
        {
            if (FD_ISSET((*socket)->fd, &read_fds))
            {
                char buffer[65536];
                size_t len = read((*socket)->fd, buffer, sizeof(buffer));
                if (len > 0)
                {
                    buffer[len]  = 0;
                    fprintf(stderr, "Socket %lx Read %d bytes from %d '%*s'\n", (unsigned long)(&**socket), (int)len, (*socket)->fd, (int)len, buffer);
                    (*socket)->on_data(buffer, len);
                }
                else
                {
                    (*socket)->fd = -1;
                    fprintf(stderr,"%03d: read of %d bytes\n", (*socket)->fd, (int)len);
                    perror("Short read");
                }
            }
            if (FD_ISSET((*socket)->fd, &write_fds))
            {
                size_t len = write((*socket)->fd,
                                   (*socket)->queuedWrite.data(), (*socket)->queuedWrite.size());
                if (len > 0)
                {
                    if (len < (*socket)->queuedWrite.size())
                    {
                        (*socket)->queuedWrite.erase(0, len);
                    }
                    else
                    {
                        (*socket)->emitDrain = true;
                        (*socket)->queuedWrite.clear();
                    }
                }
            }
            if ((*socket)->emitDrain)
            {
                if ((*socket)->on_drain)
                {
                    (*socket)->on_drain();
                }
                (*socket)->emitDrain = false;
            }

        }

        for (ssize_t i = 0; i < (ssize_t)(sockets.size()); ++i)
        {
            if (sockets[i]->fd < 0)
            {
                sockets.erase(sockets.begin() + i);
                --i;
            }
        }
        

        for (size_t i = 0; i < sockets.size(); ++i)
        {
            if (servers[i]->fd < 0)
            {
                servers.erase(servers.begin() + i);
                --i;
            }
        }
        /* DO THE WRITE, AND THE CLOSE */
	}

    releasesignals();
}

static bool AddStringUntilWhitespace(std::string &str, const char **data, size_t &length )
{
    bool nextState = false;
    size_t i;
    for (i = 0; i < length && !isspace((*data)[i]); ++i)
    {}

    str.append((*data), i);
    if (i < length) nextState = true;
    
    (*data) += i;
    length -= i;

    return nextState;
}
void HTTPRequest::ConsumeLeadingWhitespace(const char **data, size_t &length)
{
    while (length && isspace(**data))
    {
        length--;
        (*data)++;
    }
    if (!isspace(**data))
    {
        readState = &HTTPRequest::ReadHTTPMethod;
    }
}


void HTTPRequest::ReadHTTPMethod(const char **data, size_t &length)
{
    if (AddStringUntilWhitespace(method, data, length))
        readState = &HTTPRequest::ConsumeHTTPMethodWhitespace;
}

void HTTPRequest::ConsumeHTTPMethodWhitespace(const char **data, size_t &length)
{
    path.clear();
    while (length && isspace(**data))
    {
        if (**data == '\r'
            || **data == '\n')
            THROWEXCEPTION("Unexpected newline after HTTP Method");
        length--;
        (*data)++;
    }
    if (length)
        readState = &HTTPRequest::ReadHTTPPath;
}

void HTTPRequest::ReadHTTPPath(const char **data, size_t &length)
{
    if (AddStringUntilWhitespace(path,data,length))
        readState = &HTTPRequest::ConsumeHTTPPathWhitespace;
}

void HTTPRequest::ConsumeHTTPPathWhitespace(const char **data, size_t &length)
{
    while (length && isspace(**data))
    {
        if (**data == '\r'
            || **data == '\n')
            THROWEXCEPTION("Unexpected newline after HTTP Path");

        length--;
        (*data)++;
    }
    if (length)
        readState = &HTTPRequest::ReadHTTPProtocol;
}

void HTTPRequest::ReadHTTPProtocol(const char **data, size_t &length)
{
   if (AddStringUntilWhitespace(protocol,data,length))
        readState = &HTTPRequest::ConsumeHTTPProtocolNewline;
}

void HTTPRequest::ConsumeHTTPProtocolNewline(const char **data, size_t &length)
{
    while (**data == '\r')
    {
        length--;
        (*data)++;
    }
    if (**data == '\n')
    { 
        length--;
        (*data)++;
        headerName.clear();
        readState = &HTTPRequest::ReadHTTPHeaderName;
    }
    else if (**data != '\r')
        THROWEXCEPTION("Expected newline after HTTP Protocol");
}

void HTTPRequest::ReadHTTPHeaderName(const char **data, size_t &length)
{
    if (**data == '\r')
    {
        length--;
        (*data)++;
    }
    else if (**data == '\n')
    {
        length--;
        (*data)++;
        if (!headerName.empty())
            EmitNameValue(headerName, headerValue);
        readState = &HTTPRequest::ReadHTTPRequestData;
    }
    else if (isspace(**data))
    {
        if (headerName.empty())
            THROWEXCEPTION("Continuation of HTTP header with no header name");
        readState = &HTTPRequest::ReadHTTPHeaderValue;
    }
    else
    {
        if (!headerName.empty())
            EmitNameValue(headerName, headerValue);
        headerName.clear();
        headerValue.clear();
        readState = &HTTPRequest::ReadHTTPHeaderNameContinue;
    }
}
void HTTPRequest::ReadHTTPHeaderNameContinue(const char **data, size_t &length)
{
    size_t i = 0;
    for (i = 0; i < length && (*data)[i] != ':'; ++i)
    {
        if (isspace((*data)[i]))
            THROWEXCEPTION("Unexpected whitespace in HTTP header name");
    }
    if ((*data)[i] == ':')
    {
        headerName.append(*data, i);
        headerValue.clear();
        ++i;
        readState = &HTTPRequest::ConsumeHTTPHeaderNameWhitespace;
    }
    else
    {
        headerName.append(*data, i);
    }
    length -= i;
    *data += i;
}
void HTTPRequest::ConsumeHTTPHeaderNameWhitespace(const char **data, size_t &length)
{
    while (length && isspace(**data))
    {
        if (**data == '\r'
            || **data == '\n')
            THROWEXCEPTION("Unexpected newline after HTTP Path");

        length--;
        (*data)++;
    }
    if (length)
        readState = &HTTPRequest::ReadHTTPHeaderValue;
}

void HTTPRequest::ReadHTTPHeaderValue(const char **data, size_t &length)
{
    size_t i = 0;
    for (i = 0; i < length && (*data)[i] != '\r' && (*data)[i] != '\n'; ++i)
    {
    }
    if (i)
    {
        headerValue.append(*data, i);
        *data += i;
        length -= i;
    }
    if (**data == '\r')
    {
        (*data)++;
        length--;
    }
    if (**data == '\n')
    {
        (*data)++;
        length--;
        readState = &HTTPRequest::ReadHTTPHeaderName;
    }

}

void HTTPRequest::ReadHTTPRequestData(const char **data, size_t &length)
{
    // Should forward this data to the handler for this request.
    // Should also keep track of bytes sent against header info for multipart
    *data += length;
    length = 0;
}


void HTTPRequest::ReadData(const char *data, size_t length)
{
    while (length > 0)
    { 
        size_t oldLength(length);
        void (HTTPRequest::*oldReadState)(const char **data, size_t &length) (readState);
        const char *oldData(data);
    
        ((this)->*(readState))(&data, length);
        if (oldLength - length != (size_t)(data - oldData))
            THROWEXCEPTION("Read state error: data & length out of sync");
        if (length == oldLength && (readState == oldReadState))
            THROWEXCEPTION("Read state infinite loop detected");
    }
}


void HTTPRequest::EmitNameValue(const std::string &name, const std::string &value)
{
    std::cout << "Name: " << name << " value " << value << std::endl;
}

void HTTPRequest::ResetReadState()
{
    method.clear();
    path.clear();
    protocol.clear();
    headerName.clear();
    headerValue.clear();

    readState = &HTTPRequest::ConsumeLeadingWhitespace;
    
}

HTTPRequest::HTTPRequest() 
    :
    BaseObj(BASEOBJINIT(HTTPRequest)),
    readState(),
    method(),
    path(),
    protocol()
{
    ResetReadState();
}
              
