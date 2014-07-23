#include "wikivideoflash.h"

const string & VideoflashNode::Name()
{
    static string n("Videoflash");
    return n;
}

void VideoflashNode::AsHTML(HTMLOutputter &outputter)
{
    outputter.AddString("<iframe width=\"420\" height=\"315\" src=\"//www.youtube.com/embed/");
    outputter.AddString(text);
    outputter.AddString("\" frameborder=\"0\" allowfullscreen></iframe>");
}

