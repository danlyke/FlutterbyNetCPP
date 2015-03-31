
#include <sys/types.h>
#include <string.h>
#include <regex.h>
#include <stdio.h>
#include <locale.h>
#include <assert.h>

#include "fbystring.h"

const char *test = "This is a test\r\nof some CR removal\r\n\r\nTo see how things work\r\rSo there";
const char *result = "This is a test\nof some CR removal\n\nTo see how things work\n\nSo there";

int main(int /* argc */, char ** /* argv */)
{
    char * buffer = new char[strlen(test) + 1];
    strcpy(buffer,test);
    size_t length = RemoveCRs(buffer, strlen(buffer) + 1);
    assert(length == strlen(result) + 1);
    assert(!strcmp(buffer, result));
    return 0;
}
