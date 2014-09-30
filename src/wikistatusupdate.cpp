#include "treebuilder.h"
#include "wiki.h"
#include "wikistatusupdate.h"
#include "wikidb.h"
#include "parsetree.h"
#include "regexparser.h"

#include <boost/iostreams/stream.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace boost::iostreams ;

StatusUpdateNode::StatusUpdateNode(WikiDBPtr wikidb) : 
    WikiEmptyNode(BASEOBJINIT(StatusUpdateNode)),
    wikidb(wikidb)
{
}

const string & StatusUpdateNode::Name()
{
    static string n("statusupdate");
    return n;
}

void StatusUpdateNode::AsHTML(HTMLOutputter &outputter)
{
    auto attr_xid(attrs.find("xid"));
    auto attr_from(attrs.find("from"));
    auto attr_to(attrs.find("to"));

    vector< pair <string, string> > emptyAttrs;
    string strDiv("div");
    string strHeadingTag("h2");
    vector< pair <string, string> > divAttrs;
    divAttrs.push_back(pair<string,string>(string("class"),
                                           string("statusupdate")));

    vector< pair <string, string> > styleClearLeftAttrs;
    divAttrs.push_back(pair<string,string>(string("style"),
                                           string("clear: left")));
    string strDt("dt");
    string strStrong("strong");
    string strA("a");
    string strDd("dd");
    string strDl("dl");

    outputter.AddHTMLNodeBegin(strDiv,
                               divAttrs, false);

    

    if (!text.empty())
    {
        outputter.AddHTMLNodeBegin(strHeadingTag, emptyAttrs, false);
        outputter.AddString(text);
        outputter.AddHTMLNodeEnd(strHeadingTag);
    }

    vector<StatusUpdatePtr> updates;

    if (attr_xid != attrs.end())
    {
        wikidb->LoadStatusUpdates(updates, attr_xid->second);
    }
    if (attr_from != attrs.end() && attr_to != attrs.end())
    {
        wikidb->LoadStatusUpdates(updates, attr_from->second, attr_to->second);
    }
    if (!updates.empty())
    {

        for (auto update = updates.begin(); update != updates.end(); ++update)
        {
            
            TreeBuilder treeBuilder;
            MarkedUpTextParser treeParser;
            outputter.AddHTMLNodeBegin(strDt, styleClearLeftAttrs, false);
            outputter.AddHTMLNodeBegin(strStrong, emptyAttrs, false);
            outputter.AddString( (*update)->person);
            outputter.AddHTMLNodeEnd(strStrong);
            vector< pair <string, string> > linkAttrs;
            linkAttrs.push_back(pair<string,string>(string("href"),
                                                    string("/status/id/"+(*update)->xid)));
            outputter.AddHTMLNodeBegin(strA, linkAttrs, false);
            outputter.AddString(TimeToTextDate((*update)->entered));
            outputter.AddHTMLNodeEnd(strA);
            outputter.AddHTMLNodeEnd(strDt);
            outputter.AddHTMLNodeEnd(strDl);

            bool outputimage = false;
            stringstream status;
            
            if (!(*update)->imagename.empty())
            {
                ImagePtr image;
                if (wikidb->LoadImage(image, (*update)->imagename))
                {
                    status << "[[Image:";
                    status << (*update)->imagename;
                    status << "|thumb|left]] ";
                    outputimage = true;
                }
            }
            status << (*update)->status.c_str();
            string statusstr(status.str());
            treeParser.Parse(treeBuilder, statusstr.c_str(), statusstr.length());
            treeBuilder.AsHTML(outputter);
            if (outputimage) outputter.AddString("<br clear=\"left\" />");
            outputter.AddString("</dd>\n");
        }
        outputter.AddString("</dl>");

    }

    outputter.AddHTMLNodeEnd(strDiv);
    outputter.AddString("\n");
}

