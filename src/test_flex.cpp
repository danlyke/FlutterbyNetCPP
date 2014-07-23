
#include "flextest.h"
#include <iostream>
#include <map>
#include <string>

using namespace std;


TextParse::TextParse() : yyFlexLexer(&std::cin, &std::cout)
{
}


bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

int main(int /* argc */, char ** /* argv */)
{
    InitParserTokenNames();

    TextParse *tp = new TextParse();
    int res;
    do
    {
        res = tp->yylex();
        string s(tp->TokenName());

        while (replace(s, string("\n"), string("\\n"))) {}
        while (replace(s, string("\r"), string("\\r"))) {}
        
        std::cout << "Got :" << res << " (" << parserTokenNames[res] << ") '" << s << "'" << std::endl;
    } while (res > 0);
    delete tp;
    return 0;
}
