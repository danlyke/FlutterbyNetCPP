#ifndef WIKISTATUSUPDATE_H_INCLUDED
#define WIKISTATUSUPDATE_H_INCLUDED

#include "wikinode.h"

class StatusUpdateNode : public WikiEmptyNode {
protected:
    WikiDBPtr wikidb;
public:
    StatusUpdateNode(WikiDBPtr wikidb);
    virtual const string & Name();
    virtual void AsHTML(HTMLOutputter &outputter);
};

#endif /* #ifndef WIKISTATUSUPDATE_H_INCLUDED */
