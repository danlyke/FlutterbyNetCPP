#include "fbyfilefind.h"

namespace fs = boost::filesystem;

void FindFileNames(string_and_time_map_t &stagingFiles, const char *dir, const char *extension)
{
    FileFind(dir,
             [&stagingFiles, extension](fs::directory_iterator dir_iter)
             {
                 if (dir_iter->path().filename().extension() == extension)
                 {
                     std::string path(dir_iter->path().filename().string());
                     stagingFiles.insert(string_and_time_map_t::value_type(path, fs::last_write_time(*dir_iter)));
                 }
             });
}
