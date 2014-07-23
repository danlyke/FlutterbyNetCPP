#ifndef FLEXTEST_H_ONCE
#define FLEXTEST_H_ONCE

#pragma once

#include "parsertokens.h"
#include <map>
#include <string>
#include <vector>

// Only include FlexLexer.h if it hasn't been already included
#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

class TextParseToken
{
public:
    virtual const std::string AsHTML() = 0;
    virtual const std::map<std::string,std::string> &Attributes();
    virtual ~TextParseToken() {}
};

class TextParseNode : TextParseToken
{
    
};


class TextParse : public yyFlexLexer {
public:
    TextParse();
    int yylex();


    const char *TokenName() { return yytext; }
};


#endif /* #ifndef FLEXTEST_H_ONCE */
