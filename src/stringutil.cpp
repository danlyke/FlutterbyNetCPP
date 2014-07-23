#include <sys/types.h>
#include <string.h>

size_t RemoveCRs(char *buffer, size_t length)
{
    char lastchar = buffer[0];

    for (size_t i = 1; i < length; ++i)
    {
        if (lastchar == '\r' &&  buffer[i] == '\r')
        {
            buffer[i - 1] = '\n';
            buffer[i] = '\n';
        }
        else
            lastchar = buffer[i];
    }
    
    size_t follower = 0;
    for (size_t leader = 0; leader < length; ++leader)
    {
        if (buffer[leader] =='\r')
        {
        }
        else
        {
            buffer[follower] = buffer[leader];
            follower++;
        }
    }
    if (follower != length)
    {
        memset(buffer + follower, 0, length - follower);
    }
    return follower;
}

