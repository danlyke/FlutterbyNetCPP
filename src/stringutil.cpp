#include <sys/types.h>
#include <string.h>
#include "stringutil.h"

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


std::string ConvertImageNameToDescription(const std::string &name)
{
    std::string result;

    if (name.length() == 0)
        return result;

    size_t pos = 1;
    result += name[0];
    char prevChar = name[0];

    while (pos < name.length() && name[pos] != '.')
    {
        if (isalpha(name[pos]))
        {
            if (!isalpha(prevChar))
            {
                    result += ' ';
            }
            else if (isupper(name[pos]) && !isupper(prevChar))
            {
                result += ' ';
            }
        }
        else if (pos && (isalpha(prevChar)))
        {
            result += ' ';
        }

        result += name[pos];
        prevChar = name[pos];
        ++pos;
    }
    return result;
}

