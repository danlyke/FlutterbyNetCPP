#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <locale.h>
#include "fbyregexparser.h"
#include "fbywikistrings.h"
#include "fbystring.h"


class RegexMatcher : public Regex {
public:
    RegexMatcher(const char *regexName, const char *regexString)
        : Regex(regexName, regexString)
    {
    }

    virtual void ProcessMatches(TreeBuilder &treeBuilder, const char *buffer, size_t buffer_length,
                                RegexMatch &match) = 0;

    virtual ~RegexMatcher() {}
};


class RegexParagraphQuotedTextPreformat : public RegexMatcher {
public:
    RegexParagraphQuotedTextPreformat() :
        RegexMatcher("ParagraphQuotedTextPreformat",
                     "^(([^\\n\\:]+\\:\\ *\\n)?( *\\w{0,3} *[\\>])([^\\n]*)(\\n\\3[^\\n]*)+)\\n*$")
    {
    }
    virtual void ProcessMatches(TreeBuilder &treeBuilder,const char *buffer, size_t buffer_length,
                                RegexMatch &match) override;
};

class RegexParagraphCodePreformat : public RegexMatcher {
public :
    RegexParagraphCodePreformat() :
        RegexMatcher("ParagraphCodePreformat",
                     "^(\\#|\\/\\*)\\ ")
    {
    }
    virtual void ProcessMatches(TreeBuilder &treeBuilder,const char *buffer, size_t buffer_length,
                                RegexMatch &match) override;
};


class RegexParagraphList : public RegexMatcher {
private:
    Regex regexListBreaker;
public:
    RegexParagraphList(const char *name, const char *regex, const char *listregex) :
        RegexMatcher(name, regex), 
        regexListBreaker("List Breaker", listregex)
    {
        regexListBreaker.Compile();
    }
    virtual void ProcessMatches(TreeBuilder &treeBuilder,
                               const char *buffer, size_t buffer_length,
                                RegexMatch &match) override;
protected:
    virtual const char *ListTag() { return "ol"; }
    virtual char ListType() { return '\0'; };
    virtual bool HasStart() { return true; }
    virtual int ReplaceWithGroup() { return 4; }
};






Regex regexDoctypeFinder("DOCTYPE Finder",
                             "^(.*?)(\\<\\!DOCTYPE\\s+(.*?)\\s*>)");
Regex regexCommentFinder("Comment Finder",
                             "^(.*?)(\\<\\!\\-\\-\\s*(.*?)\\s*\\-\\-\\>)");
Regex regexElementFinder("Link Finder",
                             "^(.*?)\\<(\\/?)([a-z]\\w*)(.*?)(\\/?)(\\>)");
Regex regexAttrMatch1("Attribute Match 1",
                             "^\\s*([a-z]+\\w*)\\s*\\=\\s*\\\"([^\\\"]*)(\\\"\\s*)");
Regex regexAttrMatch2("Attribute Match 2",
                             "^\\s*([a-z]+\\w*)\\s*\\=\\s*(\\S+)(\\s*)");
Regex regexAttrMatch3("Attribute Match 3",
                             "^\\s*([a-z]+\\w*)\\s*\\=\\s*\\\"([^\\\"]*)(\\\"?\\s*)");


Regex regexUnderscoreQuoting1("Underscore Quoting",
"^(|.*?\\s)_(\\w|[\\w\\'\\\"][^_]*?[\\w\\'\\\"\\?\\!\\)])_\\s*((|\\'s|[\\(\\,\\.\\?\\:\\;\\!])(|[\\s\\(].*))$");

// Regex regexURLLink("URL Link",


Regex regexWikiMarkup("Wiki Markup",
                             "^(.*?)\\[((\\[(.*?)\\])|(((https?|ftp):.*?)\\s+(.*?)))(\\])");

Regex regexURL("URL",
               "^(.*?|.*?[^\"])((https?|ftp):[^\"\\)\\s ]*[^\",.\\)\\s ])");

struct {
    const char from;
    const char *to;
} entityReplacement[] = {
    { '&',  "&amp;" },
    { '<',  "&lt;" },
    { '>',  "&gt;" },
    { '"',  "&quot;" },
    { '\'', "&apos;" }
};
       

Regex regexIsExistingEntity("Is Existing Entity",
                                   "^\\&((\\w+)|\\#(\\d+)|\\#x([0-9a-z]+))\\;");


void FormatAttribute(ParseTreeNodePtr node, const char *buffer, size_t count)
{
    string s(buffer, count);

    size_t pos = 0;
    while (pos < count)
    {
        if ('&' == buffer[pos])
        {
            RegexMatch match;
            if (regexIsExistingEntity.Match(buffer+pos, count-pos, match))
            {
                pos += match.End(0);
                continue;
            }
        }

        size_t ch;
        for (ch = 0; ch < sizeof(entityReplacement) / sizeof(*entityReplacement); ++ch)
        {
            if (buffer[pos] == entityReplacement[ch].from)
                break;
        }

        if (ch < sizeof(entityReplacement) / sizeof(*entityReplacement))
        {
            string s(buffer, pos);
            
            node->AddAttributeValue(s);
            node->AddAttributeValue(entityReplacement[ch].to);
            count -= pos + 1;
            buffer += pos + 1;
            pos = 0;
        }
        else
        {
            ++pos;
        }
    }

    if (pos)
    {
        string s(buffer, pos);
        node->AddAttributeValue(s);
    }
}


void FormatString(TreeBuilder &treeBuilder, const char *buffer, size_t count)
{
    string s(buffer, count);

    size_t pos = 0;
    while (pos < count)
    {
        if ('&' == buffer[pos])
        {
            RegexMatch match;
            if (regexIsExistingEntity.Match(buffer+pos, count-pos, match))
            {
                pos += match.End(0);
                continue;
            }
        }

        size_t ch;
        for (ch = 0; ch < sizeof(entityReplacement) / sizeof(*entityReplacement); ++ch)
        {
            if (buffer[pos] == entityReplacement[ch].from)
                break;
        }

        if (ch < sizeof(entityReplacement) / sizeof(*entityReplacement))
        {
            string s(buffer, pos);
            treeBuilder.CurrentNode()->AddText(s);
            treeBuilder.CurrentNode()->AddText(entityReplacement[ch].to);
            count -= pos + 1;
            buffer += pos + 1;
            pos = 0;
        }
        else
        {
            ++pos;
        }
    }

    if (pos)
    {
        string s(buffer, pos);
        treeBuilder.CurrentNode()->AddText(s);
    }
}

void MatchURLsForLinks(TreeBuilder &treeBuilder, const char *buffer, size_t count)
{
    RegexMatch match;
    while (regexURL.Match(buffer, count, match))
    {
        FormatString(treeBuilder, buffer, match.End(1));
        ParseTreeNodePtr node(new ElementNode("a") );
        string url(match.Match(2));
        node->AddAttribute("href");
        // node->AddAttributeValue(url);
        FormatAttribute(node, buffer + match.Start(2), match.Length(2));
        treeBuilder.Push(node);
        FormatString(treeBuilder, buffer + match.Start(2), match.Length(2));
        treeBuilder.Pop();
        
        buffer += match.End(2);
        count -= match.End(2);
    }

    FormatString(treeBuilder, buffer, count);
}

void FormatWikiText(TreeBuilder &treeBuilder, const char *buffer, size_t count)
{
    RegexMatch matchWiki;
    while (regexWikiMarkup.Match(buffer, count, matchWiki))
    {
        PrintMatches("Wiki markup", buffer, matchWiki);
        MatchURLsForLinks(treeBuilder, buffer, matchWiki.End(1));
        if (matchWiki.Start(4) != matchWiki.End(4))
        {
            string wikiString(buffer + matchWiki.Start(4), matchWiki.Length(4));
            string wikiName(wikiString);
            string wikiText(wikiString);
            
            size_t pos = wikiString.find("|");
            if (pos != string::npos)
            {
                wikiText.erase(0, pos + 1);
                wikiName.erase(pos);
            }
            ParseTreeNodePtr node(new WikiNode(wikiName));
            treeBuilder.Push(node);
            FormatString(treeBuilder, wikiText.c_str(), wikiText.length());
            treeBuilder.Pop();
        }
        else if (matchWiki.Start(5) != matchWiki.End(5))
        {
            ParseTreeNodePtr node(new ElementNode("a") );
            node->AddAttribute("href");
            // string link(buffer+matchWiki.Start(5), matchWiki.Length(5));
            // node->AddAttributeValue(link);
            FormatAttribute(node, buffer + matchWiki.Start(5), matchWiki.Length(5));
//            node->AddText(link);
            treeBuilder.Push(node);
            FormatString(treeBuilder, buffer + matchWiki.Start(6), matchWiki.Length(6));
            treeBuilder.Pop();
        }
        buffer += matchWiki.End(9);
        count -= matchWiki.End(9);
//        printf("Count is now %lu %d %d\n", count, matchWiki.Start(9), matchWiki.End(9));
    }
    MatchURLsForLinks(treeBuilder, buffer, count);
}

void FormatChunk(TreeBuilder &treeBuilder,const char *buffer, size_t count)
{
    RegexMatch matchUnderscore;
    while (regexUnderscoreQuoting1.Match(buffer, count, matchUnderscore))
    {
        FormatWikiText(treeBuilder, buffer, matchUnderscore.End(0));
        string wikiName(buffer + matchUnderscore.Start(1), matchUnderscore.Length(1));
        ParseTreeNodePtr node(new WikiNode(wikiName));
        treeBuilder.Push(node);
        MatchURLsForLinks(treeBuilder, wikiName.c_str(), wikiName.length());
        treeBuilder.Pop();

        buffer += matchUnderscore.Start(2);
        count -= matchUnderscore.Start(2);
    }
    FormatWikiText(treeBuilder, buffer, count);
}


void FormatParagraph(TreeBuilder &treeBuilder,const char *buffer, size_t count)
{
    RegexMatch match;

    while (regexElementFinder.Match(buffer, count, match))
    {
        PrintMatches("link finder ", buffer, match);
        FormatChunk(treeBuilder, buffer, match.End(1));
        bool close = (match.Start(2) != match.End(2));
        bool hasAttrs = (match.Start(4) != match.End(4));
        string tagname(buffer + match.Start(3), match.Length(3));
        size_t pos;
        for (pos = 0; pos < tagname.length() && !isspace(tagname[pos]); ++pos)
        {}
        if (pos < tagname.length())
        {
            tagname.erase(pos, string::npos);
        }

        if (close)
        {
            treeBuilder.Pop(tagname);
        }
        else
        {
            ParseTreeNodePtr node(treeBuilder.NodeFactory(tagname));
            treeBuilder.Push(node);
            if (hasAttrs)
            {
                RegexMatch attrMatch;
                const char *attrs = buffer + match.Start(4);
                int attrLength = match.Length(4);
//                printf("Attrs: %.*s\n", attrLength, attrs);

                while (regexAttrMatch1.Match(attrs, attrLength, attrMatch)
                       || regexAttrMatch2.Match(attrs, attrLength, attrMatch)
                       || regexAttrMatch3.Match(attrs, attrLength, attrMatch))
                {
                    PrintMatches("Attributes",attrs,attrMatch);
                    string attr(attrs + attrMatch.Start(1), attrMatch.Length(1));
                    treeBuilder.CurrentNode()->AddAttribute(attr.c_str());
//                    string val(attrs + attrMatch.Start(2), attrMatch.Length(2));
//                    treeBuilder.CurrentNode()->AddAttributeValue(val.c_str());
                    FormatAttribute(treeBuilder.CurrentNode(),
                                    attrs + attrMatch.Start(2), attrMatch.Length(2));
                    attrs += attrMatch.End(3);
                    attrLength -= attrMatch.End(3);
                }
                if (match.Start(5) != match.End(5))
                {
                    treeBuilder.Pop();
                }
            }
        }
        buffer += match.End(6);
        count -= match.End(6);
    }
    FormatChunk(treeBuilder, buffer, count);

}


// This is a lot like FormatParagraph above and should probably be
// combined, with a lambda or something.


void FormatHTMLChunk(TreeBuilder &treeBuilder,const char *buffer, size_t count)
{
    RegexMatch match;

 
    while (regexElementFinder.Match(buffer, count, match))
    {
        PrintMatches("link finder ", buffer, match);
        FormatString(treeBuilder, buffer, match.End(1));
        bool close = (match.Start(2) != match.End(2));
        bool hasAttrs = (match.Start(4) != match.End(4));
        string tagname(buffer + match.Start(3), match.Length(3));
        size_t pos;
        for (pos = 0; pos < tagname.length() && !isspace(tagname[pos]); ++pos)
        {}
        if (pos < tagname.length())
        {
            tagname.erase(pos, string::npos);
        }

        if (close)
        {
            treeBuilder.Pop(tagname);
        }
        else
        {
            ParseTreeNodePtr node(treeBuilder.NodeFactory(tagname));
            treeBuilder.Push(node);
            if (hasAttrs)
            {
                RegexMatch attrMatch;
                const char *attrs = buffer + match.Start(4);
                int attrLength = match.Length(4);
//                printf("Attrs: %.*s\n", attrLength, attrs);

                while (regexAttrMatch1.Match(attrs, attrLength, attrMatch)
                       || regexAttrMatch2.Match(attrs, attrLength, attrMatch)
                       || regexAttrMatch3.Match(attrs, attrLength, attrMatch))
                {
                    PrintMatches("Attributes",attrs,attrMatch);
                    string attr(attrs + attrMatch.Start(1), attrMatch.Length(1));
                    treeBuilder.CurrentNode()->AddAttribute(attr.c_str());
//                    string val(attrs + attrMatch.Start(2), attrMatch.Length(2));
//                    treeBuilder.CurrentNode()->AddAttributeValue(val.c_str());
                    FormatAttribute(treeBuilder.CurrentNode(),
                                    attrs + attrMatch.Start(2), attrMatch.Length(2));
                    attrs += attrMatch.End(3);
                    attrLength -= attrMatch.End(3);
                }
                if (match.Start(5) != match.End(5))
                {
                    treeBuilder.Pop();
                }
            }
        }
        buffer += match.End(6);
        count -= match.End(6);
    }
    FormatString(treeBuilder, buffer, count);

}

void FormatHTMLComments(TreeBuilder &treeBuilder, const char *buffer, size_t length)
{
    RegexMatch match; 
    while (regexCommentFinder.Match(buffer, length, match))
    {
        FormatHTMLChunk(treeBuilder, buffer, match.End(1));
        treeBuilder.AddComment(buffer + match.Start(2), match.Length(2));
        buffer += match.End(2);
        length -= match.End(2);
    }
}
void FormatHTML(TreeBuilder &treeBuilder, const char *buffer, size_t length)
{
    RegexMatch match; 
    while (regexDoctypeFinder.Match(buffer, length, match))
    {
        FormatHTMLComments(treeBuilder, buffer, match.End(1));
        treeBuilder.AddComment(buffer + match.Start(2), match.Length(2));
        buffer += match.End(2);
        length -= match.End(2);
    }
}



void RegexParagraphList::ProcessMatches(TreeBuilder &treeBuilder,
                                       const char *buffer, size_t buffer_length,
                                        RegexMatch & /* match */)
{
    RegexMatch subMatch;
//    bool firstMatch = true;

    // cout << "ParagraphList processing matches for tag <" << ListTag() << ">" << endl;

    if (!treeBuilder.Reuse(ListTag()))
    {
        ParseTreeNodePtr node(new ElementNode(ListTag()));
        if (ListType())
        {
            node->AddAttribute("type");
            char ach[2];
            ach[0] = ListType();
            ach[1] = '\0';
            node->AddAttributeValue(ach);
        }
        treeBuilder.Push(node);
    }
    
    while (regexListBreaker.Match(buffer, buffer_length, 
                                    subMatch))
    {
        PrintMatches("List breaker",
                     buffer,
                     subMatch);
//        if (ret >= 3)
        {
            ParseTreeNodePtr node(new ElementNode("li") );
            treeBuilder.Push(node);
            
//            printf("Adding list element '%.*s'\n", 
//                   subMatch.End(4) - subMatch.Start(4),
//                   buffer + subMatch.Start(4));
            
            FormatParagraph(treeBuilder,
                            buffer + subMatch.Start(4),
                            subMatch.End(4) - subMatch.Start(4));
            int start = subMatch.End(4);
            buffer += start;
            buffer_length -= start;
            treeBuilder.Pop();
        }
    }
    treeBuilder.Pop();
}


class RegexParagraphUnnumberedList : public RegexParagraphList {
public:
    RegexParagraphUnnumberedList() :
        RegexParagraphList
        ("ParagraphUnnumberedList",
         "^( *)([\\*])( +)([^\\n]*)(\\n(\\1\\2\\3|\\1 \\3)[^\\n]*)*\\n*$",
         "^\\n*( *)([\\*])( +)([^\\n]*(\\n(\\1 \\3)[^\\n]*)*)"
            )
    {
    }
protected:
    virtual const char *ListTag() { return "ul"; }
    virtual bool HasStart() { return false; }
    virtual int ReplaceWithGroup() { return -1; }
};

class RegexParagraphNumberedList : public RegexParagraphList {
protected:
    virtual char ListType() { return '1'; };
public:
    RegexParagraphNumberedList() :
        RegexParagraphList(
            "ParagraphNumberedList",
            "^( *)(\\d+)([\\.\\)\\:\\,\\;]\\ ).*?(\\n \\d+\\3.*?)*$",
            "^\\n*( *)(\\d+)([\\.\\)\\:\\,\\;]\\ )(.*?)((\\n *\\d+\\3.*?)*)$")
    {
    }
};

class RegexParagraphLowerRomanList : public RegexParagraphList {
protected:
    virtual char ListType() { return 'i'; };
public:
    RegexParagraphLowerRomanList() :
        RegexParagraphList
        ("ParagraphLowerRomanList",
         "^( *)([xvi]+)([\\.\\)\\:]\\ ).*?(\\n [xvi]+\\3.*?)*$",
         "^\\n*( *)([xvi]+)([\\.\\)\\:])(.*?)((\\n *[xvi]+\\3.*?)*)$")
    {
    }
};

class RegexParagraphUpperRomanList : public RegexParagraphList {
protected:
    virtual char ListType() { return 'I'; };
public:
    RegexParagraphUpperRomanList() :
        RegexParagraphList
        ("ParagraphUpperRomanList",
         "^( *)([XVI]+)([\\.\\)\\:]\\ ).*?(\\n [XVI]+\\3.*?)*$",
         "^\\n*( *)([XVI]+)([\\.\\)\\:])(.*?)((\\n *[XVI]+\\3.*?)*)$"
            )
    {
    }
};

class RegexParagraphUpperAlphaNumberedList : public RegexParagraphList {
protected:
    virtual char ListType() { return 'A'; };
public:
    RegexParagraphUpperAlphaNumberedList() :
        RegexParagraphList
        ("ParagraphUpperAlphaNumberedList",
         "^( *)([A-Z])([\\.\\)\\:]\\ )[^\\n]*?(\\n [A-Z]\\3[^\\n]*?)*$",
         "^\\n*( *)([A-Z])([\\.\\)\\:]\\ )(.*?)((\\n *[A-Z]\\3.*?)*)$"
            )
    {
    }
};

class RegexParagraphLowerAlphaNumberedList : public RegexParagraphList {
protected:
    virtual char ListType() { return 'a'; };
public:
    RegexParagraphLowerAlphaNumberedList() :
        RegexParagraphList
        ("ParagraphLowerAlphaNumberedList",
         "^( *)([a-z])([\\.\\)\\:]\\ ).*?(\\n [a-z]\\3.*?)*$",
         "^\\n*( *)([a-z])([\\.\\)\\:]\\ )(.*?)((\\n *[a-z]\\3.*?)*)$"
            )
    {
    }
};


class RegexHeader : public RegexMatcher {
public:
    RegexHeader() :
        RegexMatcher("Header",
                     "^(==+)\\s*(.*?)\\s*\\1\\n*$")
    {
    }
    virtual void ProcessMatches(TreeBuilder &treeBuilder,const char *buffer, size_t buffer_length,
                                RegexMatch &match) override;
};

class RegexParagraphBlockquoteIndent1 : public RegexMatcher {
public:
    RegexParagraphBlockquoteIndent1() :
        RegexMatcher("ParagraphBlockquoteIndent1",
                     "^( +)([^\\s][^\\n]*)(\\n\\1[^\\s][^\\n]*)*\\n*$")
    {
    }
    virtual void ProcessMatches(TreeBuilder &treeBuilder,const char *buffer, size_t buffer_length,
                                RegexMatch &match) override;
};

class RegexParagraphBlockquoteIndent2 : public RegexMatcher {
public:
    RegexParagraphBlockquoteIndent2() :
        RegexMatcher("ParagraphBlockquoteIndent2",
                     "^\\s*\\<blockquote\\s*.*?\\>(.*?)\\<\\/blockquote\\s*\\>\\s*$")
    {
    }
    virtual void ProcessMatches(TreeBuilder &treeBuilder,const char *buffer, 
                                size_t buffer_length,
                                RegexMatch &match) override;
};




void RegexParagraphQuotedTextPreformat::ProcessMatches(TreeBuilder &treeBuilder, 
                                                      const char *buffer, size_t buffer_length,
                                                       RegexMatch & /* match */)
{
    ParseTreeNodePtr node(new ElementNode("pre") );
    treeBuilder.Push(node);
    FormatParagraph(treeBuilder, buffer, buffer_length);
    treeBuilder.Pop();
}

void RegexParagraphCodePreformat::ProcessMatches(TreeBuilder &treeBuilder,const char *buffer, 
                                                        size_t buffer_length,
                                                        RegexMatch & /* match */)
{
    ParseTreeNodePtr node(new ElementNode("pre") );
    treeBuilder.Push(node);
    FormatParagraph(treeBuilder, buffer, buffer_length);
    treeBuilder.Pop();
}


void RegexHeader::ProcessMatches(TreeBuilder & treeBuilder,
                                 const char * buffer, size_t /* buffer_length */,
                                 RegexMatch &match)
{
    int indent = match.Length(1) - 1;
    if (indent > 0 && indent < 7)
    {
        char header[3];
        header[0] = 'h';
        header[1] = '1' + indent;
        header[2] = '\0';

        string sectionName(header);
        sectionName += ":" + NormalizeWikiNameToFilename(
            NormalizeWikiName(match.Match(2)));

        ParseTreeNodePtr nodeHeader(new ElementNode(header));
        
        treeBuilder.Push(nodeHeader);

        nodeHeader->AddAttribute("id");
        nodeHeader->AddAttributeValue(sectionName);

        FormatString(treeBuilder, buffer + match.Start(2), match.Length(2));
        treeBuilder.Pop();
    }
}

void RegexParagraphBlockquoteIndent1::ProcessMatches(TreeBuilder &treeBuilder,const char *buffer, size_t buffer_length, RegexMatch & /* match */)
{
    ParseTreeNodePtr nodeBlockquote(new ElementNode("blockquote") );
    if (!treeBuilder.Reuse("blockquote"))
        treeBuilder.Push(nodeBlockquote);
    ParseTreeNodePtr nodeP(new ElementNode("p"));
    treeBuilder.Push(nodeP);
    FormatParagraph(treeBuilder, buffer, buffer_length - 1);
    treeBuilder.Pop();
    treeBuilder.Pop();
}

void RegexParagraphBlockquoteIndent2::ProcessMatches(TreeBuilder &treeBuilder
                                                            ,const char *buffer, size_t buffer_length,
                                                            RegexMatch & /* match */)
{
    ParseTreeNodePtr nodeBlockquote(new ElementNode("blockquote") );

    if (!treeBuilder.Reuse("blockquote"))
        treeBuilder.Push(nodeBlockquote);
    ParseTreeNodePtr nodeP(new ElementNode("p"));
    treeBuilder.Push(nodeP);
    FormatParagraph(treeBuilder, buffer, buffer_length - 1);
    treeBuilder.Pop();
    treeBuilder.Pop();
}




RegexMatcher *paragraphRegexes[] =
{
    new RegexParagraphQuotedTextPreformat(),
    new RegexParagraphCodePreformat(),
    new RegexParagraphUnnumberedList(),
    new RegexParagraphNumberedList(),
    new RegexParagraphLowerRomanList(),
    new RegexParagraphUpperRomanList(),
    new RegexParagraphUpperAlphaNumberedList(),
    new RegexParagraphLowerAlphaNumberedList(),
    new RegexHeader(),
    new RegexParagraphBlockquoteIndent1(),
    new RegexParagraphBlockquoteIndent2()
};


void MarkedUpTextParser::HandleParagraphType(TreeBuilder &treeBuilder, const char *buffer, int length)
{
    RegexMatch match;
    size_t i;

    for (i = 0; i < sizeof(paragraphRegexes) / sizeof(*paragraphRegexes); ++i)
    {
        if (paragraphRegexes[i]->Match(buffer, length, 
                                         match))
        {
//            PrintMatches(paragraphRegexes[i]->Name(),
//                         buffer, ret,
//                         (sizeof(matches)/sizeof(*matches)), matches);

            paragraphRegexes[i]->ProcessMatches(treeBuilder,
                                                buffer, length,
                                                match);
            break;
        }
    }
    if (i == sizeof(paragraphRegexes) / sizeof(*paragraphRegexes))
    {
        ParseTreeNodePtr node(new ElementNode("p") );
        treeBuilder.Push(node);
        FormatParagraph(treeBuilder, buffer, length - 1);
        treeBuilder.Pop();
    }
}


// void TestParagraphLinkBreaker(TreeBuilder &treeBuilder)
// {
//     char buffer[] = "A <a href=\"simple\">simple attribute</a>, and a <a href=\"more\n&quot;complex&apos;\">attribute</a> and a <a href='single quoted'>attribute</a>, and.";
//     size_t count = sizeof(buffer) - 1;
// 
//     cout << "Attempting to break '" << buffer << "'" << endl;
// 
//     FormatParagraph(treeBuilder, buffer, count);
// 
//     HTMLOutputterString outputter(cout);
//     
//     treeBuilder.AsHTML(outputter);
//     cout << endl;
// }

MarkedUpTextParser::MarkedUpTextParser()
    : debug(true)
{
    for (size_t i = 0; i < sizeof(paragraphRegexes) / sizeof(*paragraphRegexes); ++i)
    {
        paragraphRegexes[i]->Compile();
    }
}

MarkedUpTextParser::~MarkedUpTextParser()
{
#if 0
    for (size_t i = 0; i < sizeof(paragraphRegexes) / sizeof(*paragraphRegexes); ++i)
    {
        paragraphRegexes[i]->Free();
        delete paragraphRegexes[i];
    }
#endif
}

void MarkedUpTextParser::ParseForPara(TreeBuilder & treeBuilder, const char *buffer, size_t length)
{
    size_t para_start = 0;
    do
    {

        while (para_start < length && '\n' == buffer[para_start])
            ++para_start;

        size_t para_end = para_start + 1;
        char lastchar = buffer[para_start];
        while (para_end <= length)
        {
            if ('\n' == lastchar && '\n' == buffer[para_end])
                break;
            lastchar = buffer[para_end];
            ++para_end;
        }
        if (para_end > para_start)
        {
//            buffer[para_end] = 0;
            HandleParagraphType(treeBuilder, buffer + para_start, para_end - para_start);
        }
        para_start = para_end + 1;
    } while (para_start < length);
}

void MarkedUpTextParser::Parse(TreeBuilder & treeBuilder, const char *buffer, size_t length)
{
    RegexMatch match;
    
    while (regexCommentFinder.Match(buffer, length, match))
    {
        ParseForPara(treeBuilder, buffer, match.End(1));
        treeBuilder.AddComment(buffer + match.Start(2), match.Length(2));
        buffer += match.End(2);
        length -= match.End(2);
    }
    ParseForPara(treeBuilder, buffer, length);

}



HTMLParser::HTMLParser()
    : debug(true)
{
    for (size_t i = 0; i < sizeof(paragraphRegexes) / sizeof(*paragraphRegexes); ++i)
    {
        paragraphRegexes[i]->Compile();
    }
}

HTMLParser::~HTMLParser()
{
#if 0
    for (size_t i = 0; i < sizeof(paragraphRegexes) / sizeof(*paragraphRegexes); ++i)
    {
        paragraphRegexes[i]->Free();
        delete paragraphRegexes[i];
    }
#endif
}

void HTMLParser::Parse(TreeBuilder & treeBuilder, const char *buffer, size_t length)
{
    RegexMatch match;
    
    while (regexCommentFinder.Match(buffer, length, match))
    {
        FormatParagraph(treeBuilder, buffer, match.End(1));
        treeBuilder.AddComment(buffer + match.Start(2), match.Length(2));
        buffer += match.End(2);
        length -= match.End(2);
    }
    FormatParagraph(treeBuilder, buffer, length);

}

