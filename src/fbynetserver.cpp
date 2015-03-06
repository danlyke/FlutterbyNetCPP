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

HTTPResponse::HTTPResponse(SocketPtr socket) :
    ::FbyHelpers::BaseObj(BASEOBJINIT(HTTPResponse)),
    socket(socket),
    wroteHeader(false)
{
}
  
struct HTTPResultCodes {
    int resultCode;
    const char *resultText;
};

const char *content_type = "Content-Type";
const char *crlf = "\r\n";
const char *colonspace = ": ";
const char *textslashhtml = "text/html";


static HTTPResultCodes resultCodes[] =
{
    { 100, "Continue" },
    { 101, "Switching Protocols" },
    { 102, "Processing" }, //  (WebDAV; RFC 2518)
    { 200, "OK" },
    { 201, "Created" },
    { 202, "Accepted" },
    { 203, "Non-Authoritative Information" }, //  (since HTTP/1.1)
    { 204, "No Content" },
    { 205, "Reset Content" },
    { 206, "Partial Content" },
    { 207, "Multi-Status" }, //  (WebDAV; RFC 4918)
    { 208, "Already Reported" }, //  (WebDAV; RFC 5842)
    { 226, "IM Used" }, //  (RFC 3229)
    { 300, "Multiple Choices" },
    { 301, "Moved Permanently" },
    { 302, "Found" },
    { 303, "See Other" }, //  (since HTTP/1.1)
    { 304, "Not Modified" },
    { 305, "Use Proxy" }, //  (since HTTP/1.1)
    { 306, "Switch Proxy" },
    { 307, "Temporary Redirect" }, //  (since HTTP/1.1)
    { 308, "Permanent Redirect" }, //  (Experimental RFC; RFC 7238)
    { 400, "Bad Request" },
    { 401, "Unauthorized" },
    { 402, "Payment Required" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
    { 405, "Method Not Allowed" },
    { 406, "Not Acceptable" },
    { 407, "Proxy Authentication Required" },
    { 408, "Request Timeout" },
    { 409, "Conflict" },
    { 410, "Gone" },
    { 411, "Length Required" },
    { 412, "Precondition Failed" },
    { 413, "Request Entity Too Large" },
    { 414, "Request-URI Too Long" },
    { 415, "Unsupported Media Type" },
    { 416, "Requested Range Not Satisfiable" },
    { 417, "Expectation Failed" },
    { 418, "I'm a teapot" }, //  (RFC 2324)
    { 419, "Authentication Timeout" }, //  (not in RFC 2616)
    { 420, "Method Failure" }, //  (Spring Framework)
    { 420, "Enhance Your Calm" }, //  (Twitter)
    { 422, "Unprocessable Entity" }, //  (WebDAV; RFC 4918)
    { 423, "Locked" }, //  (WebDAV; RFC 4918)
    { 424, "Failed Dependency" }, //  (WebDAV; RFC 4918)
    { 426, "Upgrade Required" },
    { 428, "Precondition Required" }, //  (RFC 6585)
    { 429, "Too Many Requests" }, //  (RFC 6585)
    { 431, "Request Header Fields Too Large" }, //  (RFC 6585)
    { 440, "Login Timeout" }, //  (Microsoft)
    { 444, "No Response" }, //  (Nginx)
    { 449, "Retry With" }, //  (Microsoft)
    { 450, "Blocked by Windows Parental Controls" }, //  (Microsoft)
    { 451, "Unavailable For Legal Reasons" }, //  (Internet draft)
    { 452, "Conference Not Found" },
    { 453, "Not Enough Bandwidth" },
    { 454, "Session Not Found" },
    { 455, "Method Not Valid in This State" },
    { 456, "Header Field Not Valid for Resource" },
    { 457, "Invalid Range" },
    { 458, "Parameter Is Read-Only" },
    { 459, "Aggregate operation not allowed" },
    { 460, "Only aggregate operation allowed" },
    { 461, "Unsupported transport" },
    { 462, "Destination unreachable" },
    { 463, "Key management Failure" },
    { 494, "Request Header Too Large" }, //  (Nginx)
    { 495, "Cert Error" }, //  (Nginx)
    { 496, "No Cert" }, //  (Nginx)
    { 497, "HTTP to HTTPS" }, //  (Nginx)
    { 498, "Token expired/invalid" }, //  (Esri)
    { 499, "Client Closed Request" }, //  (Nginx)
    { 499, "Token required" }, //  (Esri)
    { 500, "Internal Server Error" },
    { 501, "Not Implemented" },
    { 502, "Bad Gateway" },
    { 503, "Service Unavailable" },
    { 504, "Gateway Timeout" },
    { 505, "HTTP Version Not Supported" },
    { 506, "Variant Also Negotiates" }, //  (RFC 2295)
    { 507, "Insufficient Storage" }, //  (WebDAV; RFC 4918)
    { 508, "Loop Detected" }, //  (WebDAV; RFC 5842)
    { 509, "Bandwidth Limit Exceeded[29]" }, //  (Apache bw/limited extension)
    { 510, "Not Extended" }, //  (RFC 2774)
    { 511, "Network Authentication Required" }, //  (RFC 6585)
    { 551, "Option not supported" },
    { 598, "Network read timeout error" }, //  (Unknown)
    { 599, "Network connect timeout error" }, //  (Unknown)
};

static void WriteResultCode(SocketPtr socket, int resultCode)
{
    char achResultCode[16] = "";
    int len = snprintf(achResultCode, sizeof(achResultCode), "%d", resultCode);

    socket->write(achResultCode, len);
    size_t i;
    for (i = 0; i < (sizeof(resultCodes) / sizeof(*resultCodes)); ++i)
    {
        if (resultCodes[i].resultCode == resultCode)
            break;
    }
    if (i < (sizeof(resultCodes) / sizeof(*resultCodes)))
    {
        socket->write(" ");
        socket->write(resultCodes[i].resultText);
    }
    else
    {
        socket->write(" Unknown Status Code");
    }
}

void WriteContentTypeTextHTML(SocketPtr socket)
{
    socket->write(crlf);
    socket->write(content_type);
    socket->write(colonspace);
    socket->write(textslashhtml);
}

void WriteTwoNewLines(SocketPtr socket)
{
    socket->write(crlf);
    socket->write(crlf);
}

void HTTPResponse::writeHead( int resultCode,
           const std::map<std::string,std::string> &headers)
{
    WriteResultCode(socket, resultCode);
    bool wroteContentType(false);
    for (auto header = headers.begin(); header != headers.end(); ++header)
    {
        if (header->first == content_type)
            wroteContentType = true;
        socket->write(crlf);
      
        socket->write(header->first);
        socket->write(": ");
        socket->write(header->second);
    }
    if (!wroteContentType)
    {
        WriteContentTypeTextHTML(socket);
    }
    WriteTwoNewLines(socket);
}

void HTTPResponse::writeHead( int resultCode)
{
    writeHead(resultCode, "text/html");
}

void HTTPResponse::writeHead( int resultCode, const char *)
{
    WriteResultCode(socket, resultCode);
    WriteContentTypeTextHTML(socket);
    WriteTwoNewLines(socket);
}

void HTTPResponse::end(const char *s)
{
    write(s);
}

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
                    SocketPtr socket(new Socket);
                    socket->SetSocketServerAndFile(this, fd);
					fcntl(fd,F_SETFL,O_NONBLOCK);
                    sockets.push_back(socket);
					fprintf(stderr,"%03d: new request (%d)\n",
                            (*server)->fd, fd);
                    (*server)->create_func(socket);
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

void HTTPRequestBuilder::ConsumeLeadingWhitespace(const char **data, size_t &length)
{
    while (length && isspace(**data))
    {
        length--;
        (*data)++;
    }
    if (!isspace(**data))
    {
        readState = &HTTPRequestBuilder::ReadHTTPMethod;
    }
}


void HTTPRequestBuilder::ReadHTTPMethod(const char **data, size_t &length)
{
    if (AddStringUntilWhitespace(request->method, data, length))
        readState = &HTTPRequestBuilder::ConsumeHTTPMethodWhitespace;
}

void HTTPRequestBuilder::ConsumeHTTPMethodWhitespace(const char **data, size_t &length)
{
    request->path.clear();
    while (length && isspace(**data))
    {
        if (**data == '\r'
            || **data == '\n')
            THROWEXCEPTION("Unexpected newline after HTTP Method");
        length--;
        (*data)++;
    }
    if (length)
        readState = &HTTPRequestBuilder::ReadHTTPPath;
}

void HTTPRequestBuilder::ReadHTTPPath(const char **data, size_t &length)
{
    if (AddStringUntilWhitespace(request->path,data,length))
        readState = &HTTPRequestBuilder::ConsumeHTTPPathWhitespace;
}

void HTTPRequestBuilder::ConsumeHTTPPathWhitespace(const char **data, size_t &length)
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
        readState = &HTTPRequestBuilder::ReadHTTPProtocol;
}

void HTTPRequestBuilder::ReadHTTPProtocol(const char **data, size_t &length)
{
   if (AddStringUntilWhitespace(request->protocol,data,length))
        readState = &HTTPRequestBuilder::ConsumeHTTPProtocolNewline;
}

void HTTPRequestBuilder::ConsumeHTTPProtocolNewline(const char **data, size_t &length)
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
        readState = &HTTPRequestBuilder::ReadHTTPHeaderName;
    }
    else if (**data != '\r')
        THROWEXCEPTION("Expected newline after HTTP Protocol");
}




void HTTPRequestBuilder::GenerateHTTPResponder()
{
#if 0
    for (auto route = routes.begin();
         route != routes.end();
         ++route)
    {
        if ((*route)->wants(host, method, path))
        {
            response = (*route)->create_response(host, method, path, headers);
        }
    }
#endif
}

void HTTPRequestBuilder::ReadHTTPHeaderName(const char **data, size_t &length)
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
        GenerateHTTPResponder();
        readState = &HTTPRequestBuilder::ReadHTTPRequestBuilderData;
    }
    else if (isspace(**data))
    {
        if (headerName.empty())
            THROWEXCEPTION("Continuation of HTTP header with no header name");
        readState = &HTTPRequestBuilder::ReadHTTPHeaderValue;
    }
    else
    {
        if (!headerName.empty())
            EmitNameValue(headerName, headerValue);
        headerName.clear();
        headerValue.clear();
        readState = &HTTPRequestBuilder::ReadHTTPHeaderNameContinue;
    }
}
void HTTPRequestBuilder::ReadHTTPHeaderNameContinue(const char **data, size_t &length)
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
        readState = &HTTPRequestBuilder::ConsumeHTTPHeaderNameWhitespace;
    }
    else
    {
        headerName.append(*data, i);
    }
    length -= i;
    *data += i;
}
void HTTPRequestBuilder::ConsumeHTTPHeaderNameWhitespace(const char **data, size_t &length)
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
        readState = &HTTPRequestBuilder::ReadHTTPHeaderValue;
}

void HTTPRequestBuilder::ReadHTTPHeaderValue(const char **data, size_t &length)
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
        readState = &HTTPRequestBuilder::ReadHTTPHeaderName;
    }

}

void HTTPRequestBuilder::ReadHTTPRequestBuilderData(const char **data, size_t &length)
{
    // Should forward this data to the handler for this request.
    // Should also keep track of bytes sent against header info for multipart
    *data += length;
    length = 0;
}


void HTTPRequestBuilder::ReadData(const char *data, size_t length)
{
    while (length > 0)
    { 
        size_t oldLength(length);
        void (HTTPRequestBuilder::*oldReadState)(const char **data, size_t &length) (readState);
        const char *oldData(data);
    
        ((this)->*(readState))(&data, length);
        if (oldLength - length != (size_t)(data - oldData))
            THROWEXCEPTION("Read state error: data & length out of sync");
        if (length == oldLength && (readState == oldReadState))
            THROWEXCEPTION("Read state infinite loop detected");
    }
}


void HTTPRequestBuilder::EmitNameValue(std::string name, const std::string &value)
{
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    request->headers[name] = value;
}

void HTTPRequestBuilder::ResetReadState()
{
    readState = &HTTPRequestBuilder::ConsumeLeadingWhitespace;
    request = HTTPRequestPtr(new HTTPRequest); 
   
}

HTTPRequestBuilder::HTTPRequestBuilder(RespondToHTTPRequestFunction on_request) 
    :
    BaseObj(BASEOBJINIT(HTTPRequestBuilder)),
    on_request(on_request),
    readState(),
    request()
{
    ResetReadState();
}

HTTPRequest::HTTPRequest() :
    BaseObj(BASEOBJINIT(HTTPRequest)),
    method(),
    path(),
    protocol()
{
}

              


#if 0
NODEJS drain example
// Write the data to the supplied writable stream 1MM times.
// Be attentive to back-pressure.
function writeOneMillionTimes(writer, data, encoding, callback) {
    var i = 1000000;
    write();
    function write() {
        var ok = true;
        do {
            i -= 1;
            if (i === 0) {
                // last time!
                writer.write(data, encoding, callback);
            } else {
                // see if we should continue, or wait
                // don't pass the callback, because we're not done yet.
                ok = writer.write(data, encoding);
            }
        } while (i > 0 && ok);
        if (i > 0) {
            // had to stop early!
            // write some more once it drains
            writer.once('drain', write);
        }
    }

#endif
