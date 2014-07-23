#ifndef TREEBUILDER_H_INCLUDED
#define TREEBUILDER_H_INCLUDED
#include <algorithm>
#include <functional>
#include "parsetree.h"
FBYCLASS(HTMLOutputter);

class ParseTreeNode;


FBYCLASS(TreeBuilder) : ::FbyHelpers::BaseObj {
protected:
    ParseTreeNodePtr rootNode;
    NodeStack_t nodeStack;
    ParseTreeNodePtr lastNode;
public:
    TreeBuilder(); 
    bool HasA(const char *name);
    bool ReferencesImage();

    void ForEach(std::function<void (ParseTreeNodePtr)> f);
    void AsHTML(HTMLOutputter &outputter);
    ParseTreeNodePtr CurrentNode();
    bool Reuse(const char *name);
    void Push(ParseTreeNodePtr node);
    void Pop();
    void Pop(const string &name);
    void Graft(ParseTreeNodePtr node, int where);

    virtual ParseTreeNode *NodeFactory(const std::string & /* nodename */)
    { return NULL; };
};

#endif /* #ifndef TREEBUILDER_H_INCLUDED */
