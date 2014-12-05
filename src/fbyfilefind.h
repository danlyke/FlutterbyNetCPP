#include <boost/filesystem.hpp>
#include <limits.h>
#include <iostream>
#include <map>
#include <string>

using namespace boost::filesystem;

template <class ResultSet> void FileFind(const path &dir, ResultSet rs)
{
    directory_iterator end_iter;
    char currentdir[PATH_MAX + 1];
    currentdir[PATH_MAX] = '\0';
    getcwd(currentdir, sizeof(currentdir) - 1);

    int result = chdir(dir.string().c_str());

    if (result == 0 && exists(dir) && is_directory(dir))
    {
        
        for (directory_iterator dir_iter(dir); dir_iter != end_iter; ++dir_iter)
        {
            if (is_directory(*dir_iter))
            {
                if (dir_iter->path().filename().string()[0] != '.')
                    FileFind(dir_iter->path(), rs);
            }
            else 
            {
                rs(dir_iter);
            }
        }
    }
    chdir(currentdir);
}


template <class ResultSet> void FileFind(const char *startPath, ResultSet rs)
{
    directory_iterator end_iter;
    path dir(startPath);
    FileFind(dir, rs);
}


typedef std::map<std::string, std::time_t> string_and_time_map_t;
void FindFileNames(string_and_time_map_t &stagingFiles, const char *dir, const char *extension = "");
