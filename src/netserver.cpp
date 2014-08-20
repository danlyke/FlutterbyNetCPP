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

void Socket::write(char const *data, size_t size)
{
    fprintf(stderr, "Writing %*s to %d\n", (int)size, data, fd);
    ssize_t bytes_written = ::write(fd, data, size);

    fprintf(stderr, "Wrote %d\n", (int)bytes_writted);
    if (bytes_written > 0)
    { 
        while (size > 0)
        {
            size -= bytes_written;
            data += bytes_written;
            bytes_written = ::write(fd, data, size);
        }
    }
    else
    {
        perror("Error writing");
    }
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
                    fprintf(stderr, "Read %d bytes from %d '%*s'\n", (int)len, (*socket)->fd, (int)len, buffer);
                    (*socket)->on_data(buffer, len);
                }
                else
                {
                    (*socket)->fd = -1;
                    fprintf(stderr,"%03d: read of %d bytes\n", (*socket)->fd, (int)len);
                    perror("Short read");
                }
            }

            for (size_t i = 0; i < sockets.size(); ++i)
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
        }

        /* DO THE WRITE, AND THE CLOSE */
	}

    releasesignals();
}
