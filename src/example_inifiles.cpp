#include "irrigation.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>


using namespace std;

int main()
{
    using boost::property_tree::ptree;
    ptree pt;

    read_ini("../sampledata/test.ini", pt);

    for (auto& section : pt)
    {
        std::cout << '[' << section.first << "]\n";
        for (auto& key : section.second)
            std::cout << key.first << "=" << key.second.get_value<std::string>() << "\n";
    }

    cout << endl << endl;
    cout << "Cat2.UsagePage=" << pt.get<std::string>("Cat_2.UsagePage") << std::endl;
}
