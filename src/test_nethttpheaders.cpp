#include "fbynet.h"
#include "fby.h"
#include "gtest/gtest.h"


using namespace std;
using namespace FbyHelpers;
const char requestText[] =
    "GET / HTTP/1.1\r\n"
    "Host: home.flutterby.net\r\n"
    "\r\n";

// TEST(HeaderTest,ParseRequest)
int main(int, char**)
{

    try
    {
        HTTPRequestPtr request(new HTTPRequest);
        const char *s = requestText;
        while (*s)
        {
            request->ReadData(s,1);
            ++s;
        }
        cout << "Method: " << request->method << endl;
        cout << "Path: " << request->path << endl;
        cout << "Protocol: " << request->protocol << endl;
      
//        EXPECT_EQ(request->method, string("GET"));
//        EXPECT_EQ(request->path, string("/"));
//        EXPECT_EQ(request->protocol, string("HTTP/1.1"));
    }
    catch(  FbyBaseExceptionPtr e)
    {
        cout << "<b><i>Error: ";
        cout << e->file << ":" << e->line << " " << e->Message;
        cout << "</i></b><br />\n";
        return 1;
    }
    
    try
    {
        HTTPRequestPtr request(new HTTPRequest);
        request->ReadData(requestText,sizeof(requestText)-1);
//        EXPECT_EQ(request->method, string("GET"));
//        EXPECT_EQ(request->path, string("/"));
//        EXPECT_EQ(request->protocol ,string("HTTP/1.1"));
    }
    catch(  FbyBaseExceptionPtr e)
    {
        cout << "<b><i>Error: ";
        cout << e->file << ":" << e->line << " " << e->Message;
        cout << "</i></b><br />\n";
        return 1;
    }
}
