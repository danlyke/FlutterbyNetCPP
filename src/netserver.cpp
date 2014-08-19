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

static void catchsignal_HUP(int sig)
{
	got_sighup = 1;
}


struct SignalCatch
{
    int sig;
    void (*signal_handler)(int signal);
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
}  signal_actions[sizeof(signal_actions)/sizeof(*signal_actions)]


void catchsignals()
{
    for (int i = 0; i < sizeof(signal_handlers) / sizeof(signal_handlers[0]); ++i)
    {
        memset(&signal_actions[i].new_signal_action,0,sizeof(signal_handlers[i].new_signal_action));
        sigemptyset(&signal_actions[i].new_signal_action.sa_mask);
        signal_actions[i].new_signal_action.sa_handler = signal_handlers[i].handler;
        sigaction(signal_handlers[i].sig, &signal_actions[i].new_signal_action, 
                  &signal_actions[i].old_signal_action);
    }
}

void releasesignals()
{
    for (int i = 0; i < sizeof(signal_handlers) / sizeof(signal_handlers[0]); ++i)
    {
        sigaction(signal_handlers[i].sig, &signal_actions[i].old_signal_action, 
                  &signal_actions[i].new_signal_action);
    }
}

void
Net::loop()
{
	int current_connections = 0;
	time_t now;
	struct timeval      tv;
	int                 max_fd,length;
	fd_set              read_fds,write_fds;
	for (;servers.size() && sockets.size() && !termsig;) 
	{
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
                int fd = accept(listensocket, NULL, NULL);
				if (-1 == socket->fd)
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
					fprintf(stderr,"%03d: new request (%d)\n", fd);
				}
			}
		}
		
        for (auto socket = sockets.begin(); socket != sockets.end(); ++socket)
        {
            if (FD_ISSET((*socket)->fd, &read_fds))
            {
                size_t len = read(buffer, sizeof(buffer), (*socket)->fd);
                if (len > 0)
                {
                    
                }
                else
                {
                    fprintf(stderr,"%03d: read of %d bytes\n", (*socket)->fd, (int)len);
                    perror("Short read");
                }
            }
        }

        /* DO THE WRITE, AND THE CLOSE */
	}
    releasesignals();
}
