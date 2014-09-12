#include "fbyfilefind.h"
#include <stdio.h>
#include <fstream>

using namespace std;
namespace fs = boost::filesystem;

void CopyChangedFiles(const string &source_dir, const string &target_dir)
{
    FileFind(source_dir,
             [source_dir, target_dir](fs::directory_iterator dir_iter)
             {
                 string source_filename(dir_iter->path().filename().string());
                 string target_filename(dir_iter->path().filename().string());
                 size_t pos = target_filename.rfind("/");
                 if (pos != string::npos)
                 {
                     target_filename = target_filename.erase(0, pos);
                 }
                 target_filename = target_dir + target_filename;


                 file_status target_status(status(target_filename));

                 if (exists(target_status) 
                     && last_write_time(target_filename) >= last_write_time(source_filename))
                     return;

                 FILE *source_file = fopen(source_filename.c_str(), "r");
                 if (source_file == NULL)
                 {
                     cerr << "Unable to open " << source_filename << " for reading" << endl;
                 }
                 else
                 {
                     long source_length;
                     fseek(source_file, 0L, SEEK_END);
                     source_length = ftell(source_file);
                     
                     bool do_the_copy = false;
                     char *source_bytes = new char[source_length];
                     fseek(source_file, 0L, SEEK_SET);
                     fread(source_bytes, source_length, 1, source_file);
                     fclose(source_file);
                     
                     long target_length = -1;
                     FILE *target_file = fopen(target_filename.c_str(), "r");
                     if (NULL != target_file)
                     {
                         fseek(target_file, 0L, SEEK_END);
                         target_length = ftell(target_file);
                         
                         if (target_length == source_length)
                         {
                             char *target_bytes = new char [target_length];
                             fseek(target_file, 0L, SEEK_SET);
                             fread(target_bytes, target_length, 1, target_file);
                             if (memcmp(target_bytes, source_bytes, target_length))
                                 do_the_copy = true;
                             delete[] target_bytes;
                         }
                         else
                             do_the_copy = true;
                         
                         fclose(target_file);
                     }
                     else
                         do_the_copy = true;
                     
                     if (do_the_copy)
                     {
//                         cout << "Copying " << source_filename << " to " << target_filename << endl;
                         target_file = fopen(target_filename.c_str(), "w");
                         if (NULL == target_file)
                         {
                             cerr << "Unable to open "
                                  << target_filename.c_str()
                                  << " for writing" << endl;
                         }
                         fwrite(source_bytes, source_length, 1, target_file);
                         fclose(target_file);
                     }
                     else
                     {
//                         cout <<  "Skipping " << source_filename << endl;
                     }
                     delete[] source_bytes;
                 }
             });
}
