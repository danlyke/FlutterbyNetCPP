#ifndef STRINGUTIL_H_INCLUDED
#define STRINGUTIL_H_INCLUDED
#include <vector>
#include <sstream>

inline std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


inline std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

size_t RemoveCRs(char *buffer, size_t length);

#endif /* #ifndef STRINGUTIL_H_INCLUDED */
