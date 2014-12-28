#ifndef PARSETREE_H_INCLUDED
#define PARSETREE_H_INCLUDED
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/exception/exception.hpp>
#include <assert.h>
#include "fby.h"


using namespace std;
using namespace boost;


FBYCLASS(HTMLOutputter) : public ::FbyHelpers::BaseObj
{
public:
    virtual void AddString(const string &);
    virtual void AddHTMLNodeBegin(const string &, const vector< pair<string,string> > &, bool /* empty node */);
/*
        string contents("<" + name);
        if (!attributes.empty())
        {
            for (vector< pair<string,string> >::iterator attr = attributes.begin();
                 attr != attributes.end(); ++ attr)
            {
                contents += " " + attr->first + "=\"" + attr->second + "\"";
            }
        }

*/
    virtual void AddHTMLNodeEnd(const string &);
/*             contents += "</" + name + ">";
 */
    virtual void AddWikiLink(const string &, const string &);
    virtual void AddDPLList(const std::string &category,
                            const std::string &order,
                            const std::string &pattern,
                            const std::string &count);


    virtual ~HTMLOutputter() {}
    HTMLOutputter();
};

FBYCLASS(ParseTreeNode);
FBYCLASSPTR(ParseTreeNode);


FBYCLASS(ParseTreeNode) : public ::FbyHelpers::BaseObj
{
private:
    static string name;
public:
    virtual const string & Name()
    { return name; }
    virtual void AddChild(ParseTreeNodePtr child) = 0;

    virtual void AsHTML(HTMLOutputter &outputter) { string s; outputter.AddString(s); }
    virtual void AddText(const string & /* text */) { };
    virtual void AddText(const char *text) { string s(text); AddText(s); }
    virtual ~ParseTreeNode();

    virtual void AddAttribute(const char * /* name */)
    {
    }
    virtual void AddAttributeValue(string /* value */)
    {
    }
    
    virtual void ForEachChild(std::function<void (ParseTreeNode *)> /* f */ )
    {
    }

    virtual void ForEachChild(std::function<bool (ParseTreeNode *)> /* f */ )
    {
    }

    virtual void ForEach(std::function<void (ParseTreeNode *)> f)
    {
        f(this); 
    }

    virtual void ForEach(std::function<bool (ParseTreeNode *)> f)
    {
        f(this); 
    }

    virtual string GetText();
    virtual string GetAttribute(const char * /* name */);

    virtual bool HasChildren() { return false; }
    virtual void Insert(ParseTreeNodePtr /* node */, int position);
    virtual ParseTreeNodePtr GetLastChild();
    virtual bool ReferencesImage() { return false; }
    ParseTreeNode(const char *name, int size);
};


typedef std::vector< ParseTreeNodePtr > NodeStack_t;
typedef std::vector< ParseTreeNodePtr > NodeList_t;


class TextNode : public ParseTreeNode {
protected:
    static string name;
    string contents;
public:
    TextNode();
    virtual string GetText();
    virtual const string & Name();
    virtual void AsHTML(HTMLOutputter &outputter);
    void AddText(const string &text);
    virtual void AddChild(ParseTreeNodePtr /*child*/);
};


class ElementNode : public ParseTreeNode {
protected:
    string name;
    vector< pair<string,string> > attributes;
    NodeList_t children;
    bool lastNodeIsText;
    
public:
ElementNode(const string &name_) : 
    ParseTreeNode(BASEOBJINIT(ElementNode)), 
        name(name_), 
        attributes(),
        children(),
        lastNodeIsText(false)
        {};
    virtual const string & Name()
    { return name; }
    virtual void AsHTML(HTMLOutputter &outputter);

    virtual void ForEachChild(std::function<void (ParseTreeNode *)> f) override;
    virtual void ForEach(std::function<void (ParseTreeNode *)> f) override;

    virtual void ForEachChild(std::function<bool (ParseTreeNode *)> f) override;
    virtual void ForEach(std::function<bool (ParseTreeNode *)> f) override;

    void AddText(const string &text);
    virtual void AddChild(ParseTreeNodePtr child);
    virtual void AddAttribute(const char *name);
    virtual void AddAttributeValue(string value);
    virtual bool HasChildren() override { return !children.empty(); }
    virtual bool ReferencesImage() override { return name == "img"; }
    virtual ParseTreeNodePtr GetLastChild() override;
    virtual string GetAttribute(const char *name) override;
    virtual void Insert(ParseTreeNodePtr /* node */, int position);
};

class WikiNode : public ParseTreeNode {
protected:
    static string name;
    string wikiName, wikiText;
public:
    WikiNode(const string &wikiName_);
    virtual const string & Name()
    { return name; }

    virtual void AsHTML(HTMLOutputter &outputter) { outputter.AddWikiLink(wikiName, wikiText); }
    void AddText(const string &text) { wikiText = wikiText + text; }
    virtual void AddChild(ParseTreeNodePtr /* child */);
    virtual bool ReferencesImage() override { return wikiName.length() > 6 &&  wikiName.substr(0,6) == "Image:"; }
};


bool PopNodeStackToTagname(NodeStack_t &nodeStack, const char *name);
string DecodeNamedEntity(const char *str);
string DecodeNumericEntity(const char *str);
string DecodeHexNumericEntity(const char *str);
bool PopNodeStackToTagname(NodeStack_t &nodeStack, const string &name);





#endif /* #ifndef PARSETREE_H_INCLUDED */
