#pragma once
#ifndef REGEXMATCH_H_INCLUDED
#define REGEXMATCH_H_INCLUDED
#include "fby.h"
#include <pcre.h>
#include <string>
#include <map>

class RegexMatch {
private:
    int matches[30];
    int match_count;
    const char *chars;
    size_t char_length;
    void SetCount(int m);
    void SetChars(const char *s, size_t l);
    friend class Regex;
public:
    int BufferLength();
    int *Buffer();
    int Count();
    int Start(int match);
    int End(int match);
    int Length(int match);
    std::string Match(int match);
    bool HasMatch(int match);
};



class Regex {
private:
    FBYUNCOPYABLE(Regex);
protected:
    const char *regexName;
    const char *regexString;
    pcre *regex;
    pcre_extra *regexExtra;
    int regexOptions;
public:
Regex(const char *regexName, const char *regexString, bool caseless = true)
        : regexName(regexName), regexString(regexString),
        regex(NULL), regexExtra(NULL),
        regexOptions(PCRE_DOLLAR_ENDONLY | PCRE_DOTALL)
    {
        if (caseless) regexOptions |= PCRE_CASELESS;
    }

Regex(const char *regexString, bool caseless = true)
        : regexName("<anonymous>"), regexString(regexString),
        regex(NULL), regexExtra(NULL),
        regexOptions(PCRE_DOLLAR_ENDONLY | PCRE_DOTALL)
    {
        if (caseless) regexOptions |= PCRE_CASELESS;
    }
    void Compile();

    void Free() { if (regexExtra) pcre_free_study(regexExtra); regexExtra = NULL; };

    bool Match(const char *buffer, int length, RegexMatch &match);
    bool Match(const std::string &str, RegexMatch &match) { return Match(str.c_str(), str.length(), match); }
    virtual ~Regex() { Free(); }
    const char *Name() { return regexName; }
};

void PrintMatches(const char *desc, const char *buffer, 
                  RegexMatch &match);

std::string subst(const char *input, size_t input_length,
                  const std::map<std::string,std::string> &replace);
std::string subst(const char *input,
                  const std::map<std::string,std::string> &replace);
std::string subst(const std::string &input,
                  const std::map<std::string,std::string> &replace);

std::string subst(const std::string &input,
                  const Regex &regex,
                  const std::string &replacement);
std::string subst(const char *input,
                  const Regex &regex,
                  const std::string &replacement);
std::string subst(const char *input, size_t input_length,
                  const Regex &regex,
                  const std::string &replacement);

std::vector<std::string> &split(Regex &regex, std::string str, 
                                std::vector<std::string> &elems);
inline std::vector<std::string> split(Regex &regex, std::string str)
{
    std::vector<std::string> elems;
    split(regex, str, elems);
    return elems;
}
#endif /* ifndef REGEXMATCH_H_INCLUDED */
                  
