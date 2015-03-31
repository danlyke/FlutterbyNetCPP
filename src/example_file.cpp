#include <iostream>
#include <string>
#include <map>
#include <boost/filesystem.hpp>

#include "fbyfilefind.h"

using namespace std;
namespace fs = boost::filesystem;
//fs::path sourceDir("/home/danlyke/websites/flutterby.net/code/mvs/");
//fs::path targetDir("/websites/flutterby.net/code/html_staging/");
typedef map<string, std::time_t> result_set_t;
fs::directory_iterator end_iter;

//                 # .stem!

string dotWiki(".wiki");

string NormalizeWikiNameToFilename(string wikiname)
{
    string filename(wikiname);
    bool collapse = 0;

    for (size_t i = 0; i < filename.length(); ++i)
    {
        if (filename[i] == ':')
        {
            collapse = 1;
        }
        else if (isspace(filename[i]))
        {
            if (collapse)
            {
                filename.erase(i, 1);
            }
            else
            {
                filename[i] = '_';
            }
            collapse = 1;
        }
        else
        {
            collapse = 0;
        }
    }
    return filename;
}

class FileNameAndTime
{
public:
    result_set_t &rs;
    FileNameAndTime(result_set_t &_rs) : rs(_rs) {};
    void operator ()(fs::directory_iterator /* dir_iter */)
    {
    }
};


class CompareWithStaging
{
public:
    result_set_t &rs;
    CompareWithStaging(result_set_t &_rs) : rs(_rs) {};
    void operator ()(fs::directory_iterator dir_iter)
    {
        string wikiname(dir_iter->path().filename().stem().string());
        string filename(NormalizeWikiNameToFilename(wikiname));
        string destname(filename + ".html");
        result_set_t::iterator source = rs.find(destname);
                
        if (rs.end() != source)
        {
            std::time_t t(fs::last_write_time(*dir_iter));
            cout << wikiname << " as " << filename << " comparing " << t << " to " << source->second << endl;
        }
    }
};


int main(int /* argc */, char ** /* argv */)
{
    fs::path stagingDir("/home/danlyke/websites/flutterby.net/html_staging/");
    fs::path targetDir("/var/www/");
    fs::path sourceDir("/home/danlyke/websites/flutterby.net/mvs/");

    result_set_t stagingFiles;
    FileFind("/home/danlyke/websites/flutterby.net/code/html_staging/",
             [](fs::directory_iterator dir_iter)
             {
                 cout << dir_iter->path().filename().stem().string() << endl;
             });
    return 0;

    FileFind("/home/danlyke/websites/flutterby.net/html_staging/",
             [&stagingFiles](fs::directory_iterator dir_iter)
             {
                 string path(dir_iter->path().filename().string());
                 size_t pos = path.rfind("/");
                 if (pos != string::npos)
                 {
                     path = path.erase(0, pos);
                 }
                 stagingFiles.insert(result_set_t::value_type(path, fs::last_write_time(*dir_iter)));
             }
        );

    cout << "Staging dir loaded" << endl;

    FileFind("/home/danlyke/websites/flutterby.net/mvs", [stagingFiles](fs::directory_iterator dir_iter)
             {
                 string wikiname(dir_iter->path().filename().stem().string());
                 string filename(NormalizeWikiNameToFilename(wikiname));
                 string destname(filename + ".html");
                 result_set_t::const_iterator source = stagingFiles.find(destname);
                
                 if (stagingFiles.end() != source)
                 {
                     std::time_t t(fs::last_write_time(*dir_iter));
                     cout << wikiname << " as " << filename << " comparing " << t << " to " << source->second << endl;
                 }
             });
//    FileFind("/home/danlyke/websites/flutterby.net/mvs", CompareWithStaging(stagingFiles));

    return 0;

}
