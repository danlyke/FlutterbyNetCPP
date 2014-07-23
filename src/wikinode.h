#ifndef WIKINODE_H_INCLUDED
#define WIKINODE_H_INCLUDED
#include "treebuilder.h"

class WikiEmptyNode : public ParseTreeNode {
protected:
    std::string lastAttribute;
    std::string text;
    std::map<std::string, std::string> attrs;

public:
WikiEmptyNode(const char *name, size_t size) :
    ParseTreeNode(name,size),
        lastAttribute(),
        text(),
        attrs()
        {}
    void AddText(const string &text);
    virtual void AddAttribute(const char *name);
    virtual void AddAttributeValue(string value);
    virtual bool HasChildren() override { return false; }
    virtual void AddChild(ParseTreeNodePtr /* child */) override {}
};

#endif /* #ifndef WIKINODE_H_INCLUDED */
