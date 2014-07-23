#ifndef WIKISTRINGS_H_INCLUDED
#define WIKISTRINGS_H_INCLUDED
#include <string>
#include <map>

std::string NormalizeWikiName(const std::string &inputname);
std::string NormalizeWikiNameToFilename(const std::string &wikiname);
std::string FilenameToWikiName(const std::string &inputname);
std::string VarSubst(const std::string &input,
                     std::map<std::string,std::string> vars);

#endif /* ifndef WIKISTRINGS_H_INCLUDED */
