#ifndef REGEXPARSER_INCLUDED_H
#define REGEXPARSER_INCLUDED_H

#include "fbyregex.h"
#include "parsetree.h"
#include "treebuilder.h"

class TreeParser
{
public:
    bool debug;
    void HandleParagraphType(TreeBuilder &treeBuilder, const char *buffer, int length);
    TreeParser();
    ~TreeParser();
    void Parse(TreeBuilder & treeBuilder, const char *buffer, size_t length);
protected:
    void ParseForPara(TreeBuilder & treeBuilder, const char *buffer, size_t length);
};

#endif /* #ifndef REGEXPARSER_INCLUDED_H */
