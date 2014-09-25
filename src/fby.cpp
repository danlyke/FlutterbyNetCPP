#include "fbyregex.h"
#include "wiki.h"
#include <boost/program_options.hpp>
namespace po = boost::program_options;


#include <iostream>
#include <fstream>
#include <iterator>
#include <exception>
using namespace std;

// A helper function to simplify the main part.
template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
    copy(v.begin(), v.end(), ostream_iterator<T>(os, " ")); 
    return os;
}


int main(int ac, char* av[])
{
    try {
//        int opt;
        string config_file;

        string target_file;
        string input_directory;
        string staging_directory;
        string output_directory;
        string google_maps_api_key;


        // Declare a group of options that will be 
        // allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()
            ("version,v", "print version string")
            ("help,h", "produce help message")
            ("config,c", po::value<string>(&config_file)->default_value(getenv("HOME") + string("/.fby/config.ini")),
                  "name of a file of a configuration.")
            ("doeverything", "rebuild the web site")
            ("dowikifiles", "just do the wiki files")
            ("dodirtyfiles", "just do the wiki files")
            ("dochangedfiles", "just do the wiki files")
            ("scanwikifiles", "just do the wiki files")
            ("scanimages", "just do the images files")
            ("scandplfiles", "just do the wiki files")
            ("rebuilddetached", "Fork and do everything")
            ("teststuff", "Random hook to hang weird stuff off")

            ("getwikifiles", "get a list of all of the wiki files")
            ("writewikifile", po::value<string>(&target_file),
             "wiki file to write to")
            ("writeimagefile", po::value<string>(&target_file),
             "image file to write to")
            ("readwikifile", po::value<string>(&target_file),
             "wiki file to read from")
            ("loadexifdata", po::value<string>(&target_file),
             "load a single image instance data")
            ("verifywikilink", po::value<string>(&target_file),
             "get the wiki link text out of a link")
            ("showwikistatus", po::value<string>(&target_file),
             "get the status record for a wiki entry")
            ("loadjpegdata", po::value<string>(&target_file),
             "get the status record for a wiki entry")
            ("scanwikifile", po::value<string>(&target_file),
             "scan specified file for links")
            ("dowikifile", po::value<string>(&target_file),
             "do specified file for links")
            ("markcontentdirty", po::value<string>(&target_file),
             "mark the specified wiki page dirty")
            ("markreferencesdirty", po::value<string>(&target_file),
             "mark the specified wiki page dirty")
            ;
        // Declare a group of options that will be 
        // allowed both on command line and in
        // config file
        po::options_description config("Configuration");
        config.add_options()
            ( "input_directory", po::value< string >(&input_directory),
              "directory where the .wiki files reside" )
            ( "staging_directory", po::value< string >(&staging_directory),
              "directory where we stage the HTML fiels" )
            ( "html_directory", po::value< string >(&output_directory),
              "final output directory, where the web server serves from" )
            ( "google_maps_api_key", po::value< string >(&google_maps_api_key),
              "Google maps API key" )
            ;

        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("input-file", po::value< vector<string> >(), "input file")
            ;

        
        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        po::options_description config_file_options;
        config_file_options.add(config).add(hidden);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);
        
        po::positional_options_description p;
        po::variables_map vm;
        store(po::command_line_parser(ac, av).
              options(cmdline_options).positional(p).run(), vm);
        notify(vm);

        ifstream ifs(config_file.c_str());
        if (!ifs)
        {
            cout << "can not open config file: " << config_file << "\n";
            return 0;
        }
        else
        {
            store(parse_config_file(ifs, config_file_options), vm);
            notify(vm);
        }
    
        if (vm.count("help")) {
            cout << visible << "\n";
            return 0;
        }

        if (vm.count("version")) {
            cout << "Multiple sources example, version 1.0\n";
            return 0;
        }

//        cout << "target_file : " << target_file << endl;
//        cout << "input_directory : " << input_directory << endl;
//        cout << "staging_directory : " << staging_directory << endl;
//        cout << "output_directory : " << output_directory << endl;
//        cout << "google_maps_api_key : " << google_maps_api_key << endl;

        WikiPtr wiki(FBYNEW Wiki(FBYNEW FbyPostgreSQLDB("dbname='flutterbynet' user = 'danlyke' password = 'danlyke'")));
        wiki->SetInputDirectory(input_directory);
        wiki->SetStagingDirectory(staging_directory);
        wiki->SetOutputDirectory(output_directory);
        wiki->SetGoogleMapsAPIKey(google_maps_api_key);

        if (vm.count("doeverything")) {
            wiki->DoEverything();
        }
        if (vm.count("dowikifiles")) {
            wiki->DoWikiFiles();
        }
        if (vm.count("getwikifiles")) {
            wiki->GetWikiFiles();
        }
        if (vm.count("writewikifile")) {
            wiki->WriteWikiFile(target_file);
        }
        if (vm.count("writeimagefile")) {
            wiki->WriteImageFile(target_file);
        }
        if (vm.count("readwikifile")) {
            wiki->ReadWikiFile(target_file);
        }
        if (vm.count("rebuilddetached")) {
            wiki->RebuildDetached();
        }
        if (vm.count("importkml")) {
            wiki->ImportKML(target_file);
        }
        if (vm.count("dodirtyfiles")) {
            wiki->DoDirtyFiles();
        }
        if (vm.count("dochangedfiles")) {
            wiki->DoChangedFiles();
        }
        if (vm.count("scanimages")) {
            wiki->ScanImages();
        }
        if (vm.count("scanwikifiles")) {
            wiki->ScanWikiFiles();
        }
        if (vm.count("teststuff")) {
            wiki->TestStuff();
        }
        if (vm.count("scanwikifile")) {
            wiki->ScanWikiFileForLinks(target_file.c_str());
        }
        if (vm.count("dowikifile")) {
            wiki->DoWikiFile(target_file);
        }
        if (vm.count("markcontentdirty")) {
            wiki->MarkContentDirty(target_file.c_str());
        }
        if (vm.count("markreferencesdirty")) {
            wiki->MarkReferencesDirty(target_file.c_str());
        }
        if (vm.count("loadexifdata")) {
            wiki->LoadEXIFData(target_file);
        }
        if (vm.count("scandplfiles")) {
            wiki->ScanDPLFiles();
        }
        if (vm.count("verifywikilink")) {
            wiki->VerifyWikiLink(target_file);
        }
        if (vm.count("showwikistatus")) {
            wiki->ShowWikiStatus(target_file);
        }
        if (vm.count("loadjpegdata")) {
            wiki->LoadJPEGData(target_file);
        }


    }
    catch(std::exception& e)
    {
        cout << e.what() << "\n";
        return 1;
    }
    catch(  FbyBaseExceptionPtr e)
    {
        cout << e->file << ":" << e->line << " " << e->Message << endl;
        return 1;
    }

    return  0;
}

