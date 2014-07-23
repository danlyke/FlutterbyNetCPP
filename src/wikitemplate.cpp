// FbySection:: preamble
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
// FbySection: CommandNoArgument
            ("$cmd", "$help")
// FbySection: CommandOneArgument
            ("$cmd", po::value<string>(&$arg),
             "$help")
// FbySection: CommandsEnd
            ;
        // Declare a group of options that will be 
        // allowed both on command line and in
        // config file
        po::options_description config("Configuration");
        config.add_options()
// FbySection: ConfigOption
            ( "$cmd", po::value< string >(&$option),
              "$help" )
// FbySection: ConfigEnd
            ;

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

// FbySection: Instatiation
        $typeptr wiki($new $type);
// FbySection: SetOption
        wiki->$optionmethod($optionvariable);
// FbySection: DispatchMethod
        if (vm.count("doeverything")) {
            wiki->DoEverything();
        }
// FbySection: EndDispatch
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

