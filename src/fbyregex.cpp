#include "fbyregex.h"

int RegexMatch::BufferLength()
{
    return sizeof(matches) / sizeof(*matches);
}

int *RegexMatch::Buffer()
{
    return matches;
}

void RegexMatch::SetChars(const char *s, size_t l)
{
    chars = s;
    char_length = l;
}

void RegexMatch::SetCount(int m)
{
    match_count = m;
}


int RegexMatch::Count()
{
    return match_count;
}


int RegexMatch::Start(int match)
{
    if (!(match < match_count))
        assert(match < match_count);
    return matches[match * 2];
}


int RegexMatch::End(int match)
{
    if (!(match < match_count))
        assert(match < match_count);
    return matches[match * 2 + 1];
}

bool RegexMatch::HasMatch(int match)
{
    return match >= 0 && match < match_count && matches[match * 2] != -1;
}

int RegexMatch::Length(int match)
{
    return End(match) - Start(match);
}


std::string RegexMatch::Match(int match)
{
    std::string s(chars + Start(match), Length(match));
    return s;
}




void PrintMatches(const char *desc, const char *buffer, 
                  RegexMatch &match)
{
    return;
    printf("Matched (%d): %s\n", match.Count(), desc);

    for (int j = 0; j < match.Count(); ++j)
    {
        if (match.Length(j) != 0)
        {
            printf("Print Matches %d: (%2d,%2d): '%.*s'\n", j, 
                   match.Start(j),
                   match.End(j),
                   match.Length(j),
                   buffer + match.Start(j));
        }
    }
}


void PrintRegcompError(const char *name, const char *error, int offset)
{
    printf("Error compiling %s: %s at %d\n", name, error, offset);
}

void Regex::Compile()
{
    const char *pcreErrorStr;
    int pcreErrorOffset;

    regex = pcre_compile(regexString, regexOptions,
                         &pcreErrorStr, &pcreErrorOffset, NULL);
                         
    if (NULL == regex)
    {
        PrintRegcompError(regexName, pcreErrorStr, pcreErrorOffset);
    }
    regexExtra = pcre_study(regex, 0, &pcreErrorStr);
    if (pcreErrorStr != NULL)
        printf("ERROR: Could not study %s: %s\n", regexName, pcreErrorStr);
}

bool Regex::Match(const char *buffer, int length, RegexMatch &match)
{
    if (NULL == regex) Compile();
    match.SetChars(buffer, length);
    int pcreExecRet = pcre_exec(regex, 
                                regexExtra,
                                buffer, length,
                                0, // start looking here
                                0, // OPTIONS
                                match.Buffer(), match.BufferLength());
    if (pcreExecRet < 0)
    {
        switch(pcreExecRet) {
        case PCRE_ERROR_NOMATCH      :
//            printf("String did not match the pattern\n");
            break;
        case PCRE_ERROR_NULL         :
            printf("%s: Something was null\n", regexName);
            break;
        case PCRE_ERROR_BADOPTION    :
            printf("%s: A bad option was passed\n", regexName);
            break;
        case PCRE_ERROR_BADMAGIC     :
            printf("%s: Magic number bad (compiled re corrupt?)\n", regexName);
            break;
        case PCRE_ERROR_UNKNOWN_NODE :
            printf("%s: Something kooky in the compiled re\n", regexName);
            break;
        case PCRE_ERROR_NOMEMORY     :
            printf("%s: Ran out of memory\n", regexName);
            break;

        default                      :
            printf("%s: Unknown error (%d)\n", regexName, pcreExecRet);
            break;
        } /* end switch */
    }
    match.SetCount(pcreExecRet);
    return pcreExecRet >= 0;
}



Regex findVar("^(.*?)\\$(\\w+)");
std::string subst(const char *input,
                  const std::map<std::string,std::string> &replace)
{
    return subst(input, strlen(input), replace);
}

std::string subst(const std::string &input,
                  const std::map<std::string,std::string> &replace)
{
    return subst(input.data(), input.length(), replace);
}

std::string subst(const char *input, size_t input_length,
                  const std::map<std::string,std::string> &replace)
{
    std::string result;
    RegexMatch match;

    while (findVar.Match(input, input_length, match))
    {
        std::string varname = match.Match(2);
        result += match.Match(1);
        auto var_repl = replace.find(varname);
        if (var_repl == replace.end())
        {
            result += "$" + varname;
        }
        else
        {
            result += var_repl->second;
        }
        input += match.End(2);
        input_length -= match.End(2);
    }
    result += std::string(input, input_length);
    return result;
}


std::vector<std::string> &split(Regex &regex, std::string str, std::vector<std::string> &elems)
{
    RegexMatch match;
    const char *data = str.data();
    size_t length = str.length();

    while (length > 0 && regex.Match(data, length, match))
    {
        elems.push_back(std::string(data, match.Start(0)));
        data += match.End(0);
        length -= match.End(0);
    }
    if (length > 0)
    {
        elems.push_back(std::string(data, length));
    }
    return elems;
}

