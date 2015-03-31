#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include "fbytreebuilder.h"
#include "fbystring.h"
#include "fbyregexparser.h"
#include "wiki.h"



class TestTreeBuilder : public TreeBuilder
{
public:
    TestTreeBuilder() : TreeBuilder() {};
    virtual ~TestTreeBuilder() {}
    ParseTreeNode *NodeFactory(const std::string & nodename)
    {
        return new ElementNode(nodename);
    }
};


int main(int argc, char** argv) {
    TestTreeBuilder treeBuilder;

    setlocale(LC_ALL, ""); /* Use system locale instead of default "C" */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s regex string\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    fseek(f, 0L, SEEK_END);
    size_t length = ftell(f);
    fseek(f, 0L, SEEK_SET);
    
    char *buffer = new char[length + 1];
    buffer[length] = 0;
    fread(buffer, length, 1, f);
    fclose(f);

    length = RemoveCRs(buffer, length);

    {
        MarkedUpTextParser treeParser;
        treeParser.Parse(treeBuilder, buffer,length);
    }
    

    HTMLOutputterString outputter(cout);
    
    treeBuilder.AsHTML(outputter);
    cout << endl;
    return 0;
}
