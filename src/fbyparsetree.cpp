#include "fbyparsetree.h"
#include "fbywikistrings.h"

HTMLOutputter::HTMLOutputter()
    : FbyHelpers::BaseObj(BASEOBJINIT(HTMLOutputter))
{}


void HTMLOutputter::AddString(const string &) {};
void HTMLOutputter::AddHTMLNodeBegin(const string &, const vector< pair<string,string> > &, bool /* empty node */) {};
void HTMLOutputter::AddHTMLNodeEnd(const string &) {};
void HTMLOutputter::AddWikiLink(const string &, const string &) {};
void HTMLOutputter::AddDPLList(const std::string &/* category */,
                               const std::string &/* order */,
                               const std::string &/* pattern */,
                               const std::string &/* count */) {};


string ParseTreeNode::name("Base node class");
string TextNode::name("Text node");
string WikiNode::name("Wiki Node");

ParseTreeNodePtr ParseTreeNode::GetLastChild()
{
    assert(0); ParseTreeNodePtr n(NULL); return n;
}
void ParseTreeNode::AddChild(ParseTreeNodePtr /*child*/)
{
    assert(0);
//        THROW_BOOST_EXCEPTION("WTF");
}

void ParseTreeNode::Insert(ParseTreeNodePtr /* node */, int /* position */)
{
}


string ParseTreeNode::GetText()
{
    string text;

    ForEachChild([&text](ParseTreeNode *node)
            {
                text += node->GetText();
            }
        );
    return text;
}
string ParseTreeNode::GetAttribute(const char * /* name */)
{
    return string("");
}


string TextNode::GetText()
{
    return contents;
}

string ElementNode::GetAttribute(const char *name)
{
    string namestr(name);
    for (auto attr = attributes.begin();
         attr != attributes.end();
         ++attr)
    {
        if (attr->first == namestr)
            return attr->second;
    }
    return string();
}


void ElementNode::Insert(ParseTreeNodePtr node, int position)
{
    children.insert(children.begin() + position, node);
}



void ElementNode::AsHTML(HTMLOutputter &outputter)
{
    outputter.AddHTMLNodeBegin(name, attributes, children.empty()); 
    for (NodeList_t::iterator node = children.begin(); node != children.end(); ++node)
    {
        (*node)->AsHTML(outputter);
    }
    if (!children.empty())
        outputter.AddHTMLNodeEnd(name);
}

void ElementNode::ForEachChild(std::function<bool (ParseTreeNode *)> f)
{
    for (auto node = children.begin(); node != children.end(); ++node)
    {
        (*node)->ForEach(f);
    }
}


void ElementNode::ForEach(std::function<bool (ParseTreeNode *)> f)
{
    if (f(this))
    {
        ForEachChild(f);
    }
}

void ElementNode::ForEachChild(std::function<void (ParseTreeNode *)> f)
{
    for (auto node = children.begin(); node != children.end(); ++node)
    {
        (*node)->ForEach(f);
    }
}


void ElementNode::ForEach(std::function<void (ParseTreeNode *)> f)
{
    f(this);
    ForEachChild(f);
}


void ElementNode::AddText(const string &text)
{
    if (!lastNodeIsText)
    {
        ParseTreeNodePtr node(new TextNode);
        children.push_back(node);
        lastNodeIsText = true;
    }
    children.back()->AddText(text);
}

void ElementNode::AddChild(ParseTreeNodePtr child)
{
    lastNodeIsText = false;
    children.push_back(child);
}
void ElementNode::AddAttribute(const char *name)
{
    attributes.push_back(pair<string,string>(string(name), string("")));;
}

void ElementNode::AddAttributeValue(string value)
{
    if (!attributes.empty())
    {
        attributes.back().second += value;
    }
}

ParseTreeNodePtr ElementNode::GetLastChild()
{
    assert(!children.empty());
    return children.back();
}

WikiNode::WikiNode(const string &wikiName)
    : ParseTreeNode(BASEOBJINIT(WikiNode)),
      wikiName(NormalizeWikiName(wikiName)), wikiText("")
{
}

void WikiNode::AddChild(ParseTreeNodePtr /* child */)
{
    assert(0);
//        THROW_BOOST_EXCEPTION("WTF");
}

void TextNode::AddChild(ParseTreeNodePtr /* child */)
{
    assert(0);
//        THROW_BOOST_EXCEPTION("WTF");
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

ParseTreeNode::ParseTreeNode(const char *name, int size) : ::FbyHelpers::BaseObj(name,size)
{
//    cout << "Creating ParseTreeNode " << name << endl;
}

ParseTreeNode::~ParseTreeNode()
{
//    cout << "Destroying ParseTreeNode " << name << endl;
};

TextNode::TextNode()
    : ParseTreeNode(BASEOBJINIT(TextNode)),
      contents()
{}

const string & TextNode::Name()
{ return name; }

void TextNode::AsHTML(HTMLOutputter &outputter) { outputter.AddString(contents); }
void TextNode::AddText(const string &text) { contents = contents + text; }
