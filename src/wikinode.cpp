#include "wikinode.h"

using namespace std;

void WikiEmptyNode::AddAttribute(const char *name)
{
    lastAttribute = string(name);
    attrs[lastAttribute] = "";
}

void WikiEmptyNode::AddAttributeValue(std::string value)
{
//    cout << "Setting " << lastAttribute << " to value " << value << endl;
    attrs[lastAttribute] += value;
}

void WikiEmptyNode::AddText(const std::string &t)
{
    text += t;
}

