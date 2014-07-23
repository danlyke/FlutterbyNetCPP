#ifndef WIKIVIDEOFLASH_H_INCLUDED
#define WIKIVIDEOFLASH_H_INCLUDED

#include "wikinode.h"

class VideoflashNode : public WikiEmptyNode {
protected:

public:
VideoflashNode() : 
    WikiEmptyNode(BASEOBJINIT(VideoflashNode))
        {};

    virtual const string & Name();

    virtual void AsHTML(HTMLOutputter &outputter);
};

#endif /* #ifndef WIKIVIDEOFLASH_H_INCLUDED */
