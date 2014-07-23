#include "flextest.h"
#include <iostream>
#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/exception/exception.hpp>
#include <assert.h>

using namespace std;
using namespace boost;


class ParseTreeNode;
typedef boost::shared_ptr<ParseTreeNode> ParseTreeNodePtr;


class ParseTreeNode {
private:
    static string name;
public:
    virtual const string & Name()
    { return name; }
    virtual void AddChild(ParseTreeNodePtr child) = 0;

    virtual string AsHTML() { string s; return s; }
    virtual void AddText(const string &text) {};
    virtual void AddText(const char *text) { string s(text); AddText(s); }
    virtual ~ParseTreeNode() {};

    virtual void AddAttribute(const char *name)
    {
    }
    virtual void AddAttributeValue(string value)
    {
    }
};

string ParseTreeNode::name("Base node class");

typedef vector< ParseTreeNodePtr > NodeStack_t;
typedef vector< ParseTreeNodePtr > NodeList_t;


class TextNode : public ParseTreeNode {
protected:
    static string name;
    string contents;
public:
    TextNode() : ParseTreeNode() {}
    virtual const string & Name()
    { return name; }

    virtual string AsHTML() { return contents; }
    void AddText(const string &text) { contents = contents + text; }
    virtual void AddChild(ParseTreeNodePtr child)
    {
        THROW_BOOST_EXCEPTION("WTF");
    }
};

string TextNode::name("Text node");

class ElementNode : public ParseTreeNode {
protected:
    string name;
    vector< pair<string,string> > attributes;
    NodeList_t children;
    bool lastNodeIsText;
    
public:
    ElementNode(const string &name_) : ParseTreeNode(), name(name_), lastNodeIsText(false) {};
    virtual const string & Name()
    { return name; }
    virtual string AsHTML()
    {
        string contents("<" + name);
        if (!attributes.empty())
        {
            for (vector< pair<string,string> >::iterator attr = attributes.begin();
                 attr != attributes.end(); ++ attr)
            {
                contents += " " + attr->first + "=\"" + attr->second + "\"";
            }
        }
        
        if (children.empty())
        {
            contents += "/>";
        }
        else
        {
            contents += ">";
            for (NodeList_t::iterator node = children.begin(); node != children.end(); ++node)
            {
                contents += (*node)->AsHTML();
            }
            contents += "</" + name + ">";
        }
        return contents;
    }

    void AddText(const string &text) 
    {
        if (!lastNodeIsText)
        {
            ParseTreeNodePtr node(new TextNode);
            children.push_back(node);
            lastNodeIsText = true;
        }
        children.back()->AddText(text);
    }
    virtual void AddChild(ParseTreeNodePtr child)
    {
        lastNodeIsText = false;
        children.push_back(child);
    }
    virtual void AddAttribute(const char *name)
    {
        attributes.push_back(pair<string,string>(string(name), string("")));;
    }
    virtual void AddAttributeValue(string value)
    {
        if (!attributes.empty())
        {
            attributes.back().second += value;
        }
    }
};

class WikiNode : public ParseTreeNode {
protected:
    static string name;
    string contents;
public:
    WikiNode() : ParseTreeNode() {};
    virtual const string & Name()
    { return name; }

    virtual string AsHTML() { return contents; }
    void AddText(const string &text) { contents = contents + text; }
    virtual void AddChild(ParseTreeNodePtr child)
    {
        THROW_BOOST_EXCEPTION("WTF");
    }
};
string WikiNode::name("Wiki Node");



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



bool PopNodeStackToTagname(NodeStack_t &nodeStack, const char *name)
{
    NodeStack_t::reverse_iterator node;
    for (node = nodeStack.rbegin(); node != nodeStack.rend(); ++node)
    {
        if ((*node)->Name() == name)
            break;
    }
    if (nodeStack.rend() == node)
        return false;
    
    while (nodeStack.back()->Name() != name)
    {
        nodeStack.pop_back();
    }
    return true;
}

string DecodeNamedEntity(const char *str)
{
    string s(str);
    return s;
}

string DecodeNumericEntity(const char *str)
{
    string s(str);
    return s;
}

string DecodeHexNumericEntity(const char *str)
{
    string s(str);
    return s;
}



bool PopNodeStackToTagname(NodeStack_t &nodeStack, const string &name)
{
    return PopNodeStackToTagname(nodeStack, name.c_str());
}


int main(int argc, char **argv)
{
    InitParserTokenNames();
    
    ParseTreeNodePtr rootNode(new ElementNode("body"));
    NodeStack_t nodeStack;
    nodeStack.push_back(rootNode);

    TextParse *tp = new TextParse();
    int res;
    do
    {
        res = tp->yylex();
        
        switch (res)
        {
        case NEW_PARAGRAPH :
        {
            if (PopNodeStackToTagname(nodeStack, "p"))
                nodeStack.pop_back();

            ParseTreeNodePtr node(new ElementNode("p"));
            nodeStack.back()->AddChild(node);
            nodeStack.push_back(node);
        }
        break;

        case ORDERED_LIST_ITEM :
        {
            if (!PopNodeStackToTagname(nodeStack, "ol"))
            {
                ParseTreeNodePtr node(new ElementNode("ol"));
                nodeStack.back()->AddChild(node);
                nodeStack.push_back(node);
            }
            {
                ParseTreeNodePtr node(new ElementNode("li"));
                nodeStack.back()->AddChild(node);
                nodeStack.push_back(node);
                const char *p = tp->TokenName();
                while (*p && isspace(*p)) { ++p; }
                while (*p && !isspace(*p)) { ++p; }
                while (*p && isspace(*p)) { ++p; }
                node->AddText(p);
            }
        }
        break;

        case UNORDERED_LIST_ITEM :
        {
            if (!PopNodeStackToTagname(nodeStack, "ul"))
            {
                ParseTreeNodePtr node(new ElementNode("ul"));
                nodeStack.back()->AddChild(node);
                nodeStack.push_back(node);
            }
            {
                ParseTreeNodePtr node(new ElementNode("li"));
                nodeStack.back()->AddChild(node);
                nodeStack.push_back(node);
                const char *p = tp->TokenName();
                while (*p && isspace(*p)) { ++p; }
                while (*p && !isspace(*p)) { ++p; }
                while (*p && isspace(*p)) { ++p; }
                node->AddText(p);
            }
        }
        break;

        case PARAGRAPH_TEXT :
            nodeStack.back()->AddText(tp->TokenName());
            break;
        case NAMED_ENTITY :
            nodeStack.back()->AddText(DecodeNamedEntity(tp->TokenName()));
            break;
        case NUMERIC_ENTITY :
            nodeStack.back()->AddText(DecodeNumericEntity(tp->TokenName()));
            break;
        case HEX_NUMERIC_ENTITY :
            nodeStack.back()->AddText(DecodeHexNumericEntity(tp->TokenName()));
            break;
        case TAG_OPEN :
        {
            ParseTreeNodePtr node(new ElementNode(tp->TokenName() + 1));
            nodeStack.back()->AddChild(node);
            nodeStack.push_back(node);
        }
        break;
        case TAG_CLOSE :
        {
            string name(tp->TokenName());
            if (boost::starts_with(name, "</"))
            {
                size_t len = name.length() - 2;
                while (!isalnum(name[len]) && len > 2)
                    --len;
                name.erase(len + 1);
                name.erase(0,2);
            }

            if (PopNodeStackToTagname(nodeStack, name))
                nodeStack.pop_back();
        }
        break;
        case TAG_ATTRIBUTE :
        {
            nodeStack.back()->AddAttribute(tp->TokenName());
            break;
        }
        case TAG_ATTRIBUTE_VALUE :
        {
            nodeStack.back()->AddAttributeValue(tp->TokenName());
        }
        break;
        case TAG_ATTRIBUTE_VALUE_CLOSED :
            break;
        case TAG_ATTRIBUTE_NAMED_ENTITY :
        {
            nodeStack.back()->AddAttributeValue(DecodeNamedEntity(tp->TokenName()));
        }
        break;
        case TAG_ATTRIBUTE_NUMERIC_ENTITY :
        {
            nodeStack.back()->AddAttributeValue(DecodeNumericEntity(tp->TokenName()));
        }
        break;
        case TAG_ATTRIBUTE_HEX_NUMERIC_ENTITY :
        {
            nodeStack.back()->AddAttributeValue(DecodeHexNumericEntity(tp->TokenName()));
        }
        break;
        case TAG_ATTRIBUTE_CLOSE :
            break;

        case WIKI_OPEN :
        {
            ParseTreeNodePtr node(new WikiNode());
            nodeStack.back()->AddChild(node);
            nodeStack.push_back(node);
        }
        break;

        case WIKI_CLOSE :
        {
            if (PopNodeStackToTagname(nodeStack, "Wiki Node"))
            {
                nodeStack.pop_back();
            }
        }
        break;

        case HEADING1 :
        case HEADING2 :
        case HEADING3 :
        case HEADING4 :
        case HEADING5 :
        case HEADING6 :
            break;

        case UNMATCHED :
            assert(0);
            break;
        }

    } while (res > 0);
    delete tp;
    cout << rootNode->AsHTML() << endl;
    return 0;
}
