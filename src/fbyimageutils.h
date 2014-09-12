#pragma once
#ifndef IMAGEUTILS_H_INCLUDED
#define IMAGEUTILS_H_INCLUDED

#include <string>
#include <map>

bool FindJPEGSize(const std::string &filename,
                  int & width, int & height,
                  std::map<std::string,std::string> &attributes);


#endif /* #ifndef IMAGEUTILS_H_INCLUDED */
