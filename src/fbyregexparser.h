#ifndef REGEXPARSER_INCLUDED_H
#define REGEXPARSER_INCLUDED_H

#include "fbyregex.h"
#include "fbyparsetree.h"
#include "fbytreebuilder.h"

class MarkedUpTextParser
{
public:
    bool debug;
    void HandleParagraphType(TreeBuilder &treeBuilder, const char *buffer, int length);
    MarkedUpTextParser();
    ~MarkedUpTextParser();
    void Parse(TreeBuilder & treeBuilder, const char *buffer, size_t length);
protected:
    void ParseForPara(TreeBuilder & treeBuilder, const char *buffer, size_t length);
};

class HTMLParser
{
public:
    bool debug;
    HTMLParser();
    ~HTMLParser();
    void Parse(TreeBuilder & treeBuilder, const char *buffer, size_t length);
};

#endif /* #ifndef REGEXPARSER_INCLUDED_H */
