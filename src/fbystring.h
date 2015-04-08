#ifndef STRINGUTIL_H_INCLUDED
#define STRINGUTIL_H_INCLUDED
#include <vector>
#include <sstream>

inline std::vector<std::string> &split(char delim, const std::string &s, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


inline std::vector<std::string> split(char delim, const std::string &s) {
    std::vector<std::string> elems;
    split(delim, s, elems);
    return elems;
}

size_t RemoveCRs(char *buffer, size_t length);
std::string ConvertImageNameToDescription(const std::string &name);

std::string HTMLQuote(const std::string &s);

bool endswith(const std::string &s, const std::string &with);
bool startswith(const std::string &s, const std::string &with);

#endif /* #ifndef STRINGUTIL_H_INCLUDED */
