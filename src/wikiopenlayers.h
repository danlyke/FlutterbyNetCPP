#ifndef WIKIOPENLAYERS_H_INCLUDED
#define WIKIOPENLAYERS_H_INCLUDED

#include "wikinode.h"

class OpenLayersNode : public WikiEmptyNode {
protected:
    static int mapNum;
    int width;
    int height;
    double lon;
    double lat;
    int zoom;

public:
OpenLayersNode() : 
    WikiEmptyNode(BASEOBJINIT(OpenLayersNode)), 
        width(640),
        height(400),
        lon(0),
        lat(0),
        zoom(13)
        {};

    virtual const string & Name();

    virtual void AsHTML(HTMLOutputter &outputter);
};

#endif /* #ifndef WIKIOPENLAYERS_H_INCLUDED */
