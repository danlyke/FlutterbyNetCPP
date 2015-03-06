#include "fbynet.h"
#include "fby.h"
#include "gtest/gtest.h"


using namespace std;
using namespace FbyHelpers;
const char requestText[] =
    "GET / HTTP/1.1\r\n"
    "Host: home.flutterby.net\r\n"
    "Another-Header: this one goes\r\n"
    "  all the way to two lines\r\n"
    "\r\n";

TEST(HeaderTest,ParseRequest)
//int main(int, char**)
{
    SocketPtr socket;
    try
    {
        HTTPRequestBuilderPtr request(new HTTPRequestBuilder(socket,
                                          [](HTTPRequestPtr, HTTPResponsePtr){}
                                          ));
        request->ReadData(requestText,sizeof(requestText)-1);
        EXPECT_EQ(request->request->method, string("GET"));
        EXPECT_EQ(request->request->path, string("/"));
        EXPECT_EQ(request->request->protocol ,string("HTTP/1.1"));
    }
    catch(  FbyBaseExceptionPtr e)
    {
        cerr << e->file << ":" << e->line << " " << e->Message << endl;
    }

    try
    {
        HTTPRequestBuilderPtr request(new HTTPRequestBuilder(socket,
                                          [](HTTPRequestPtr, HTTPResponsePtr){}
                                          ));
        const char *s = requestText;
        while (*s)
        {
            request->ReadData(s,1);
            ++s;
        }
        EXPECT_EQ(request->request->method, string("GET"));
        EXPECT_EQ(request->request->path, string("/"));
        EXPECT_EQ(request->request->protocol ,string("HTTP/1.1"));
    }
    catch(  FbyBaseExceptionPtr e)
    {
        cerr << e->file << ":" << e->line << " " << e->Message << endl;
    }

    try
    {
        HTTPRequestBuilderPtr request(new HTTPRequestBuilder(socket,
                                          [](HTTPRequestPtr, HTTPResponsePtr){}
                                          ));
        const char *s = requestText;
        request->ReadData(s,4);
        s += 4;

        while (*s)
        {
            request->ReadData(s,1);
            ++s;
        }
        EXPECT_EQ(request->request->method, string("GET"));
        EXPECT_EQ(request->request->path, string("/"));
        EXPECT_EQ(request->request->protocol ,string("HTTP/1.1"));
    }
    catch(  FbyBaseExceptionPtr e)
    { 
        cerr << e->file << ":" << e->line << " " << e->Message << endl;
    }
    
}
