#ifndef WIKIDPL_H_INCLUDED
#define WIKIDPL_H_INCLUDED

#include "wikinode.h"

class DPLNode : public WikiEmptyNode {
protected:
    static int mapNum;
    int width;
    int height;
    double lon;
    double lat;
    int zoom;

public:
DPLNode() : 
    WikiEmptyNode(BASEOBJINIT(DPLNode)), 
        width(640),
        height(400),
        lon(0),
        lat(0),
        zoom(13)
        {};

    virtual const string & Name();

    virtual void AsHTML(HTMLOutputter &outputter);
};

#endif /* #ifndef WIKIDPL_H_INCLUDED */
