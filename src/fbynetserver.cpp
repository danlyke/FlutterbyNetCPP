#include "fbynet.h"
#include "fbystring.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
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
using namespace std;

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
//    fprintf(stderr, "Writing Socket %lx %*s to %d\n", (unsigned long)(this), (int)size, data, fd);
    bool wroteEverything(false);
    ssize_t bytes_written = ::write(fd, data, size);

//    fprintf(stderr, "Wrote %d\n", (int)bytes_written);
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
    else if (bytes_written < 0 || size)
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

const char content_type[] = "Content-Type";
const char crlf[] = "\r\n";
const char colonspace[] = ": ";
const char textslashhtml[] = "text/html";
const char imageslashjpeg[] = "image/jpeg";

struct MIMETypesByExtension {
    const char *extension;
    const char *mimeType;
};


const char defaultMimeType[] = "application/octet-stream";

static MIMETypesByExtension mimeTypesByExtension[] = {
    { ".htm", 	textslashhtml },
    { ".html", 	textslashhtml },
    { ".ico", 	"image/x-icon" },
    { ".jpeg", 	imageslashjpeg },
    { ".jpg", 	imageslashjpeg },
    { ".js", 	"application/x-javascript" },
    { ".png", 	"image/png" },
    { ".txt", 	"text/plain" },
    { ".svg",	"image/svg+xml" },
    // Needs a Content-Encoding header too...
    { ".svgz",	"image/svg+xml" },
    { ".json", "application/json" },
    { ".css", "text/css" },
};

const char *MimeTypeForExtension(const string &path)
{
    const char *mimeType = defaultMimeType;
    
    for (size_t i = 0; i < sizeof(mimeTypesByExtension) / sizeof(*mimeTypesByExtension); ++i)
    {
        if (endswith(path, mimeTypesByExtension[i].extension))
        {
            mimeType = mimeTypesByExtension[i].mimeType;
            break;
        }
    }
    return mimeType;
}


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
    char achResultCode[32] = "";
    int len = snprintf(achResultCode, sizeof(achResultCode), "HTTP/1.1 %d", resultCode);

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
#if 0
    socket->write("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
#else
    WriteResultCode(socket, resultCode);
    WriteContentTypeTextHTML(socket);
    WriteTwoNewLines(socket);
#endif
}


void HTTPResponse::respondHTML(int resultCode, std::map<std::string, std::string> attrs,
                               const std::string &title, const std::string &body)
{
    attrs["Content-Type"] = "text/html; charset=utf-8";
    
    std::string html("<html><head><title>");
    html += title;
    html += "</title></head>\n<body><h1>";
    html += title;
    html += "</h1><p>" + body + "</p></body></html>\n";
    
    attrs["Content-Length"] = std::to_string(html.length());
    writeHead(resultCode, attrs);
    end(html);
}

void HTTPResponse::respondHTML(int resultCode, 
                               const std::string &title, const std::string &body)
{
    std::map<std::string, std::string> attrs;
    respondHTML(resultCode,attrs,title,body);
}

void HTTPResponse::respondHTML(int resultCode, const std::string &body)
{
    std::map<std::string, std::string> attrs;

    std::string title;
    size_t i;
    for (i = 0; i < (sizeof(resultCodes) / sizeof(*resultCodes)); ++i)
    {
        if (resultCodes[i].resultCode == resultCode)
            break;
    }
    if (i < (sizeof(resultCodes) / sizeof(*resultCodes)))
    {
        title = resultCodes[i].resultText;
    }
    else
    {
        title = to_string(resultCode);
    }

    respondHTML(resultCode,attrs,title,body);
}

void HTTPResponse::redirect(const std::string &path)
{
    std::map<std::string,std::string> attrs;
    attrs["Location"] = path;
    string redirect("The document has moved <a href=\"");
    redirect += path;
    redirect += "\">here</a>";
    respondHTML(302, attrs, "302 Found", redirect);
        
               
}


bool HTTPResponse::end(const char *data, size_t length)
{
    return socket->end(data,length);
}

bool HTTPResponse::end(const std::string &s)
{
    return socket->end(s);
}

bool HTTPResponse::end(const char *s)
{
    return socket->end(s);
}



void
Net::loop()
{
	time_t now;
    struct timeval      previous_time;
	int                 max_fd;
	fd_set              read_fds,write_fds;

//    fprintf(stderr, "Beginning of loop\n");
//    fprintf(stderr, "Server size is %d sockets size is %d\n",
//            (int)(servers.size()), (int)(sockets.size()));
    termsig = 0;
    catchsignals();

    gettimeofday(&previous_time, NULL);

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

        struct timeval current_time;
        gettimeofday(&current_time, NULL);
        time_t elapsed_seconds = current_time.tv_sec - previous_time.tv_sec;
        long elapsed_microseconds =
            (long)(current_time.tv_usec / 1000) - (long)(previous_time.tv_usec / 1000);
        elapsed_microseconds += int(elapsed_seconds) * 1000;
        previous_time = current_time;

        long next_timer_microseconds(LONG_MAX);

        for (size_t i = 0; i < timers.size(); ++i)
        {
            auto timer = timers[i];

            if (timer->next_time <= elapsed_microseconds)
            {
                timer->triggered_function();
                if (timer->microsecondsRecurring)
                {
                    timer->next_time += timer->microsecondsRecurring;
                }
                else
                {
                    timers.erase(timers.begin() + i);
                    --i;
                    continue;
                }
            }
            timer->next_time -= elapsed_microseconds;
            if (timer->next_time  < next_timer_microseconds)
            {
                next_timer_microseconds = timer->next_time;
            }
        }

        struct timeval *ptv(NULL), tv;

        if (next_timer_microseconds != LONG_MAX)
        {
            ptv = &tv;
            tv.tv_sec  = next_timer_microseconds / 1000 ;
            tv.tv_usec = (next_timer_microseconds % 1000) * 1000;
        }

		if (-1 == select(max_fd+1,
				 &read_fds,
				 &write_fds,
				 NULL,ptv))
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
					fprintf(stderr,"%03d: %ld new request (%d)\n",
                            (*server)->fd, now, fd);
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
//                    fprintf(stderr, "Socket %lx Read %d bytes from %d '%*s'\n", (unsigned long)(&**socket), (int)len, (*socket)->fd, (int)len, buffer);
                    (*socket)->on_data(buffer, len);
//                    std::cerr << "Sent bytes to " << (*socket)->fd << " done " << (*socket)->doneWithWrites << std::endl;
                }
                else
                {
                    (*socket)->on_end();
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
            SocketPtr socket = sockets[i];
            fprintf(stderr, "Checking socket %lx status %d\n", (unsigned long)(&*socket), socket->doneWithWrites);
            if (sockets[i]->doneWithWrites
                && !sockets[i]->emitDrain)
            {
                close(sockets[i]->fd);
                sockets[i]->fd = -1;
            }
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
    HTTPResponsePtr response(new HTTPResponse(socket));
    on_request(request, response);
    
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

void HTTPRequestBuilder::ReadResetReadState(const char ** /* data */, size_t & /* length */)
{
    ResetReadState();
}

void HTTPRequestBuilder::ReadHTTPRequestBuilderData(const char **data, size_t &length)
{
    string s(*data, length);
    if (content_length > length)
    {
        request->on_data(*data, length);
        *data += length;
        content_length -= length;
        length = 0;
    }
    else
    {
        request->on_data(*data, content_length);
        request->on_end();
        *data += content_length;
        length -= content_length;
        content_length = 0;
        readState = &HTTPRequestBuilder::ReadResetReadState;
     }
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
    if (name == "Content-Length")
        content_length = stoi(value);
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    request->headers[name] = value;
}


void HTTPRequestBuilder::ResetReadState()
{
    readState = &HTTPRequestBuilder::ConsumeLeadingWhitespace;
    request = HTTPRequestPtr(new HTTPRequest); 
   
}

HTTPRequestBuilder::HTTPRequestBuilder(SocketPtr socket,
                                       RespondToHTTPRequestFunction on_request) 
    :
    BaseObj(BASEOBJINIT(HTTPRequestBuilder)),
    on_request(on_request),
    content_length(-1),
    socket(socket),
    readState(),
    request(),
    headerName(),
    headerValue()
{
    ResetReadState();
}

HTTPRequest::HTTPRequest() :
    BaseObj(BASEOBJINIT(HTTPRequest)),
    on_data([](const char *, size_t){}),
    on_end(),
    method(),
    path(),
    protocol(),
    headers()
{
}




bool ServeFile(const char * fileRoot, HTTPRequestPtr request, HTTPResponsePtr response)
{
    int fd;
    string path(fileRoot);
    string filename(request->path);
    if (filename.length() == 0)
    {
        response->respondHTML(404,
                              "Not Found",
                              "Zero length filename, this ain't right");
        return true;
    }
    if (filename[0] != '/')
    {
        filename = "/" + filename;
    }
    if (string::npos != filename.find("/."))
    { 
        response->respondHTML(403,
                              "Wrist Slap",
                              "Gotta /. in your filename, not cool");
        return true;
    }

    path += filename;

    struct stat statbuf;

    if (stat(path.c_str(), &statbuf))
    { 
        response->respondHTML(404,
                              "Not Found",
                              "Error in finding <i>" + path + "</i>");
    }

    if (S_ISDIR(statbuf.st_mode))
    {
        if (path[path.length() - 1] != '/')
        {
            response->redirect(filename + '/');
            return true;
        }
        path += "index.html";
    }
    cout << "Reading " << path << endl;

    if (0 < (fd = open(path.c_str(), O_RDONLY)))
    {
        // Need to do file type stuff here!
        response->writeHead(200, MimeTypeForExtension(path));
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
              

BodyParserURLEncoded::BodyParserURLEncoded()
    :
    BaseObj(BASEOBJINIT(BodyParserURLEncoded)),
    readState(&BodyParserURLEncoded::ReadName),
    name(),
    value(),
    entity(-1),
    on_name_value([](const std::string &, const std::string &){})

{
}


void BodyParserURLEncoded::ResetReadState()
{
    name.clear();
    value.clear();
    readState = &BodyParserURLEncoded::ReadName;
}


void BodyParserURLEncoded::AppendUntil( string &which, const char toggleOn,
                            const char **data, size_t &length)
{
    size_t i;
    for (i = 0;
         i < length && (toggleOn != (*data)[i])
             && ('%' != (*data)[i])
             && ('+' != (*data)[i]);
         ++i)
    {
    }
    which.append(*data, i);
    *data += i;
    length -= i;
}

void BodyParserURLEncoded::ReadName(const char **data, size_t &length)
{
    AppendUntil(name, '=', data, length);
    if (length)
    {
        switch(**data)
        {
        case '=' :
            readState = &BodyParserURLEncoded::ReadValue;
            break;
        case '%' :
            readState = &BodyParserURLEncoded::ReadNameEntity1;
            break;
        case '+' :
            readState = &BodyParserURLEncoded::ReadNamePlusSpace;
            break;
        default:
            assert(0);
            break;
        }
        ++(*data);
        --length;
    }
}

int BodyParserURLEncoded::ReadDataAsHexDigit(const char **data, size_t &length)
{
    int result(-1);
    if (**data >= '0' && **data <= '9')
    {
        result = **data - '0';
        ++(*data);
        --length;
    }
    else if (**data >= 'A' && **data <= 'F')
    {
        result = **data - 'A' + 0xa;
        ++(*data);
        --length;
    } 
    else if (**data >= 'a' && **data <= 'f')
    {
        result = **data - 'a' + 0xa;
        ++(*data);
        --length;
    } 
    return result;
}

void BodyParserURLEncoded::ReadNamePlusSpace(const char ** /* data */, size_t & /* length */)
{
    name.append(" ");
    readState = &BodyParserURLEncoded::ReadName;
}

void BodyParserURLEncoded::ReadNameEntity1(const char **data, size_t &length)
{
    int result = ReadDataAsHexDigit(data, length);
    if (result >= 0)
    {
        entity = result << 4;
        readState = &BodyParserURLEncoded::ReadNameEntity2;
    }
    else
    {
        readState = &BodyParserURLEncoded::ReadName;
    }
}

void BodyParserURLEncoded::ReadNameEntity2(const char **data, size_t &length)
{
    int result = ReadDataAsHexDigit(data, length);
    if (result >= 0)
    {
        entity |= result;
        char ch[2];
        ch[0] = (char)(entity);
        ch[1] = '\0';
        name.append(ch);
    }
    readState = &BodyParserURLEncoded::ReadName;
}

void BodyParserURLEncoded::ReadValue(const char **data, size_t &length)
{
    AppendUntil(value, '&', data, length);
    if (length)
    {
        switch(**data)
        {
        case '&' :
            EmitNameValue(name,value);
            name.clear();
            value.clear();
            readState = &BodyParserURLEncoded::ReadName;
            break;
        case '%' :
            readState = &BodyParserURLEncoded::ReadValueEntity1;
            break;
        case '+' :
            readState = &BodyParserURLEncoded::ReadValuePlusSpace;
            break;
        default:
            assert(0);
            break;
        }
        ++(*data);
        --length;
    }
}

void BodyParserURLEncoded::ReadValuePlusSpace(const char ** /* data */, size_t & /* length */)
{
    value.append(" ");
    readState = &BodyParserURLEncoded::ReadValue;
}


void BodyParserURLEncoded::ReadValueEntity1(const char **data, size_t &length)
{
    int result = ReadDataAsHexDigit(data, length);
    if (result >= 0)
    {
        entity = result << 4;
        readState = &BodyParserURLEncoded::ReadValueEntity2;
    }
    else
    {
        readState = &BodyParserURLEncoded::ReadValue;
    }
}

void BodyParserURLEncoded::ReadValueEntity2(const char **data, size_t &length)
{
    int result = ReadDataAsHexDigit(data, length);
    if (result >= 0)
    {
        entity |= result;
        char ch[2];
        ch[0] = (char)(entity);
        ch[1] = '\0';
        value.append(ch);
    }
    readState = &BodyParserURLEncoded::ReadValue;
}

void BodyParserURLEncoded::EmitNameValue(const std::string &name, const std::string &value)
{
    on_name_value(name, value);
}

void BodyParserURLEncoded::on_data( const char *data, size_t length)
{
    while (length > 0)
    {
        ((this)->*(readState))(&data, length);
    }
}

void BodyParserURLEncoded::on_end()
{
    EmitNameValue(name,value);
    ResetReadState();
}

void BodyParserURLEncoded::onNameValue(OnNameValueFunction on_name_value)
{
    this->on_name_value = on_name_value;
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
