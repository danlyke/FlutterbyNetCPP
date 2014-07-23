%{
#include "flextest.h"
%}

%option yyclass="TextParse" noyywrap c++


%s paragraph_text
%x paragraph_code

%x tag_name
%x tag_body
%x tag_attribute
%x tag_attribute_value
%x tag_attribute_expected
%x tag_attribute_doublestring
%x tag_attribute_singlestring

%x wiki_name_contents


PARA_BREAK \n\n+

LIST_OL_BEGIN [\ \t]*[[:alnum:]]+[\.\:][\ \t]
LIST_UL_BEGIN [\ \t]*[\*\-][\ \t]
CODE_BLOCK_BEGIN [\#]

PARAGRAPH_TEXT  [^&\[<_=*\n]*
PARAGRAPH_CHAR  [&\[<_\n]
TAG             [<]{ATTRIBUTE_NAME}

ATTRIBUTE_NAME  [[:alpha:]][[:alnum:]]*
NAMED_ENTITY    \&[[:alpha:]][[:alnum:]]*\;
NUMERIC_ENTITY  \&\#[0-9]+\;
HEX_NUMERIC_ENTITY  \&\#x[0-9a-zA-Z]+\;
WIKI_OPEN       \[\[
WIKI_CLOSE      \]\]


%%

{PARA_BREAK} { return NEW_PARAGRAPH; }
{LIST_OL_BEGIN}{PARAGRAPH_TEXT} { BEGIN(paragraph_text); return ORDERED_LIST_ITEM; }
{LIST_UL_BEGIN}{PARAGRAPH_TEXT} { BEGIN(paragraph_text); return UNORDERED_LIST_ITEM; }
{PARAGRAPH_TEXT} { BEGIN(paragraph_text); return PARAGRAPH_TEXT; }
{PARAGRAPH_CHAR} { BEGIN(paragraph_text); return PARAGRAPH_TEXT; }
<paragraph_text>\=\=[[:space:]]*[^\n][[:space:]]*\=\=\n { return HEADING1; }
<paragraph_text>\=\=\=[[:space:]]*[^\n][[:space:]]*\=\=\=\n { return HEADING2; }
<paragraph_text>\=\=\=\=[[:space:]]*[^\n][[:space:]]*\=\=\=\=\n { return HEADING3; }
<paragraph_text>\=\=\=\=\=[[:space:]]*[^\n][[:space:]]*\=\=\=\=\=\n { return HEADING4; }
<paragraph_text>\=\=\=\=\=\=[[:space:]]*[^\n][[:space:]]*\=\=\=\=\=\=\n { return HEADING5; }
<paragraph_text>\=\=\=\=\=\=\=[[:space:]]*[^\n][[:space:]]*\=\=\=\=\=\=\=\n { return HEADING6; }

<paragraph_text>{WIKI_OPEN}  { yy_push_state(wiki_name_contents); return WIKI_OPEN; }

<wiki_name_contents>{WIKI_CLOSE} { yy_pop_state(); return WIKI_CLOSE; }
<wiki_name_contents>[^\]\&]+ { return PARAGRAPH_TEXT; }
<wiki_name_contents>[\]\&]   { return PARAGRAPH_TEXT; }

<wiki_name_contents,paragraph_text,tag_attribute_singlestring,tag_attribute_doublestring>NAMED_ENTITY       { return NAMED_ENTITY; }
<wiki_name_contents,paragraph_text,tag_attribute_singlestring,tag_attribute_doublestring>NUMERIC_ENTITY     { return NUMERIC_ENTITY; }
<wiki_name_contents,paragraph_text,tag_attribute_singlestring,tag_attribute_doublestring>HEX_NUMERIC_ENTITY { return HEX_NUMERIC_ENTITY; }

"<"{ATTRIBUTE_NAME}        { yy_push_state(tag_body); return TAG_OPEN; }
"</"{ATTRIBUTE_NAME}[[:space:]]*">"        { return TAG_CLOSE; }

<tag_body,tag_attribute_expected>[\r\n ]* { }
<tag_body>{ATTRIBUTE_NAME}     { BEGIN(tag_attribute_value); return TAG_ATTRIBUTE; }
<tag_attribute_value>"="       { BEGIN(tag_attribute_expected); }
<tag_attribute_value>\s*       { BEGIN(tag_body); }
<tag_attribute_value,tag_body>">"       { yy_pop_state(); return TAG_ATTRIBUTE_CLOSE; }
<tag_attribute_value,tag_body>"/>"       { yy_pop_state(); return TAG_CLOSE; }


<tag_attribute_expected>[[:space:]]*"\""   BEGIN(tag_attribute_doublestring);
<tag_attribute_expected>[[:space:]]*"'"    BEGIN(tag_attribute_singlestring);

<tag_attribute_doublestring>"\""                              { BEGIN(tag_body); return TAG_ATTRIBUTE_VALUE_CLOSED; }
<tag_attribute_doublestring>[^\"\\\&]*                        {return TAG_ATTRIBUTE_VALUE; }

<tag_attribute_singlestring>"'"                               { BEGIN(tag_body); return TAG_ATTRIBUTE_VALUE_CLOSED; }
<tag_attribute_singlestring>[^\'\\\&]*                        {return TAG_ATTRIBUTE_VALUE; }


<INITIAL,tag_attribute_doublestring,tag_attribute_singlestring,paragraph_text>{NAMED_ENTITY}        {return TAG_ATTRIBUTE_NAMED_ENTITY; }
<INITIAL,tag_attribute_doublestring,tag_attribute_singlestring,paragraph_text>{NUMERIC_ENTITY}      {return TAG_ATTRIBUTE_NUMERIC_ENTITY; }
<INITIAL,tag_attribute_doublestring,tag_attribute_singlestring,paragraph_text>{HEX_NUMERIC_ENTITY}  {return TAG_ATTRIBUTE_HEX_NUMERIC_ENTITY; }



