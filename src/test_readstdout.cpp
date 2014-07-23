#include <stdio.h>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <fstream>
#include <iostream>
using namespace std;
using namespace boost::iostreams ;

int main()
{
    FILE *f = popen("find . -type f -print", "r");
    if (NULL == f)
    {
        perror("Unable to popen find");
        return 0;
    }
#if 0
    int fd = fileno(f);
    file_descriptor_sink fds(fd, file_descriptor_flags::never_close_handle);
    boost::iostreams::stream_buffer<file_descriptor_sink> bis(fds);
    std::istream is(&bis);

    while (is) {
        std::string line;
        std::getline(is, line);
        std::cout << line << std::endl;
    }
#else
    char buffer[256];
    while (NULL != fgets(buffer, sizeof(buffer) - 1, f))
    {
        string s(buffer);
        cout << s << endl;
    }
#endif
    pclose(f);
    return 0;
}
