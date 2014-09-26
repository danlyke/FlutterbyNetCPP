#include "fbyregex.h"
#include "fby.h"
#include <iostream>
#include <fstream>
#include <map>
#include <boost/algorithm/string.hpp>
#include <string>
#include <stdio.h>

using namespace std;
using namespace FbyHelpers;

#define REGEXBLOCK_NOSQL "(\\s+(\\/\\*\\s*nosql\\s*\\*\\/)|\\/\\/\\s*nosql)?"
Regex regexMatchClass("regexMatchClass",
                 "^\\s*class\\s+(.*?)\\s*\\:\\s*public \\s*FbyORM"
    REGEXBLOCK_NOSQL);

Regex regexMatchClass1("regexMatchClass",
                 "^\\s*FBYCLASS\\s*\\(\\s*(.*?)\\s*\\)\\s*\\:\\s*public \\s*FbyORM"
    REGEXBLOCK_NOSQL);
Regex regexMatchMember("regexMatchMember",
                       "^\\s*(bool|int|float|double|long|string|std\\:\\:string|time_t)\\s+(\\w+)\\s*;\\s*//\\s*SQL\\s+(.*?)\\,?\\s*(--.*)?$");
Regex regexMatchEnd("regexMatchEnd",
                    "^\\s*\\}\\s*\\;");

Regex regexMatchLiteralSQL("literalSQL",
                           "^\\s*\\/\\/\\s+SQL\\:\\s+(.*)$");


struct MemberDescription
{
    string ctype;
    string name;
    string sqltype;
    bool hasKey;
    MemberDescription(const string &ctype,
                      const string &name,
                      const string &sqltype,
                      bool hasKey)
    : ctype(ctype), name(name), sqltype(sqltype), hasKey(hasKey)
    {
    }

};


void PrintSQLDefinition(ostream &ossql, ostream &oscpp, const string &currentClass, vector<MemberDescription> members)
{
    map<string, string> textToMember;

    textToMember["bool"] = "wrapper_stobool";
    textToMember["int"] = "wrapper_stoi";
    textToMember["long"] = "wrapper_stol";
    textToMember["long"] = "long std::stoll";
    textToMember["float"] = "wrapper_stof";
    textToMember["double"] = "wrapper_stod";
    textToMember["long"] = "double wrapper_stold";
    textToMember["string"] = "";
    textToMember["std::string"] = "";
    textToMember["time_t"] = "TextDateToTime";

    map<string, string> memberToText;

    memberToText["bool"] = "to_string";
    memberToText["int"] = "to_string";
    memberToText["long"] = "to_string";
    memberToText["long"] = "to_string";
    memberToText["float"] = "to_string";
    memberToText["double"] = "to_string";
    memberToText["long"] = "to_string";
    memberToText["string"] = "";
    memberToText["std::string"] = "";
    memberToText["time_t"] = "TimeToTextDate";

//     time_t strftime
//          
//         stringstream ss;
//     ss << membear;
//     value = ss.str();
// 
    ossql << "DROP TABLE IF EXISTS " << currentClass << /* " CASCADE;" */ ";" << endl;
    ossql << "CREATE TABLE " << currentClass << " (" << endl;
    for (vector<MemberDescription>::iterator member = members.begin(); member != members.end(); ++member)
    {
        ossql << "    " << member->name << " " << member->sqltype;

        vector<MemberDescription>::iterator isLast = member;
        isLast++;
        if (isLast != members.end()) 
        {
            ossql << ",";
        }
        ossql << endl;
    }
    ossql << ");" << endl;

    oscpp << "void " << currentClass << "::AssignToMember( const std::string &memberName, const std::string &value)" << endl;
    oscpp << "{" << endl;
    oscpp << "    std::string ucMemberName(boost::to_upper_copy(memberName));\n";
    for (vector<MemberDescription>::iterator member = members.begin(); member != members.end(); ++member)
    {
        oscpp << "    if (ucMemberName == \"" << boost::to_upper_copy(member->name) << "\") { " << member->name << " = ";
        map<string,string>::iterator mapping = textToMember.find(member->ctype);
        if (mapping != textToMember.end())
        {
            oscpp << mapping->second;
        }
        else
        {
            cerr << "Unknown c type " << member->ctype << endl;
        }
        oscpp << "(value); }";
        oscpp << " // ctype: " << member->ctype;
        oscpp << endl;
        
    }
    oscpp << "}" << endl;
    oscpp << endl;

    oscpp << "std::string " << currentClass << "::AssignFromMember( const std::string &memberName )" << endl;
    oscpp << "{" << endl; 
    oscpp << "    std::string ucMemberName(boost::to_upper_copy(memberName));\n";
    oscpp << "    string value;" << endl;
    for (vector<MemberDescription>::iterator member = members.begin(); member != members.end(); ++member)
    {
        oscpp << "    if (ucMemberName == \"" << boost::to_upper_copy(member->name) << "\") { value = ";
        map<string,string>::iterator mapping = memberToText.find(member->ctype);
        if (mapping != textToMember.end())
        {
            oscpp << mapping->second;
        }
        else
        {
            cerr << "Unknown c type " << member->ctype << endl;
        }
        oscpp << "(" << member->name << "); }" << endl;
    }
    oscpp << "    return value;" << endl;
    oscpp << "}" << endl;
    oscpp << endl;
    oscpp << "const char **" << currentClass << "::StaticMemberNames() {" << endl;
    oscpp << "    static const char *memberNames[] = {" << endl;

    for (vector<MemberDescription>::iterator member = members.begin(); member != members.end(); ++member)
    {
        oscpp << "        \""  << member->name << "\"," << endl;
    }
    oscpp << "        NULL" << endl;
    oscpp << "    };" << endl;
    oscpp << "    return memberNames;" << endl;
    oscpp << "};" << endl;
    oscpp << endl;
    oscpp << "const char *" << currentClass << "::StaticClassName() { return \"" << currentClass << "\"; }" << endl;
    oscpp << endl;

    oscpp << "const char **" << currentClass << "::StaticKeyNames() {" << endl;
    oscpp << "    static const char *keyNames[] = {" << endl;

    bool hasSecondaryKeys;

    for (vector<MemberDescription>::iterator member = members.begin(); member != members.end(); ++member)
    {
        if (member->hasKey)
            hasSecondaryKeys = true;

        if (member->sqltype.find("PRIMARY KEY") != string::npos)
        {
            oscpp << "        \"" << member->name << "\"," << endl;
        }
    }
    if (hasSecondaryKeys)
    {
        for (auto member = members.begin();
             member != members.end(); ++member)
        {
            if (member->hasKey)
            {
                oscpp << "\"" << member->name << "\", " << endl;
            }
        }
    }
    oscpp << "        NULL," << endl;
    oscpp << "    };" << endl;
    oscpp << "    return keyNames;" << endl;
    oscpp << "}" << endl;
    oscpp << endl;

    oscpp << "const char **" << currentClass
          << "::MemberNames() { return "
          << currentClass << "::StaticMemberNames(); }" << endl;
    oscpp << "const char **" << currentClass
          << "::KeyNames() { return "
          << currentClass << "::StaticKeyNames(); }" << endl;
    oscpp << "const char *" << currentClass
          << "::ClassName() { return "
          << currentClass << "::StaticClassName(); }" << endl;
    oscpp << endl;
    
}

void ParseFile(const char *filename)
{
    std::filebuf fbcpp, fbsql;
    fbcpp.open ("sqldefinitions.cpp",std::ios::out);
    fbsql.open ("sqldefinitions.sql",std::ios::out);
    std::ostream oscpp(&fbcpp);
    std::ostream ossql(&fbsql);

    RegexMatch match;
    string line;
    string currentClass;
    bool suppressSQL = false;

    vector<MemberDescription> members;
    oscpp << "#include <string>" << endl;
    oscpp << "#include <stdio.h>" << endl;
    oscpp << "#include <boost/algorithm/string.hpp>" << endl;
    oscpp << "#include \"" << filename << "\"" << endl;
    oscpp << endl;
    oscpp << endl;
    oscpp << "using namespace std;" << endl;

    try
    {
        std::ifstream istr(filename);

        while (getline(istr, line))
        {
            if (regexMatchLiteralSQL.Match(line, match))
            {

                string literalsql = match.Match(1);
                ossql << literalsql << endl;
            }

            if (regexMatchClass.Match(line, match)
                || regexMatchClass1.Match(line, match))
            {
                if (!currentClass.empty() && !suppressSQL)
                {
                    PrintSQLDefinition(ossql, oscpp, currentClass,members);
                    members.clear();
                }
                currentClass = match.Match(1);
                suppressSQL = match.HasMatch(2) && !match.Match(2).empty();
                cout << "Matched class " << currentClass << endl;
            }
            if (regexMatchMember.Match(line, match))
            {
                if (!currentClass.empty())
                {
                    string ctype = match.Match(1);
                    string name = match.Match(2);
                    string sqltype = match.Match(3);
                    bool hasKey = match.HasMatch(4);
                    members.push_back( MemberDescription(ctype,
                                                         name,
                                                         sqltype,
                                                         hasKey
                                           ) );
                }
            }
            if (regexMatchEnd.Match(line, match))
            {
                if (!members.empty() && !currentClass.empty())
                {
                    PrintSQLDefinition(ossql, oscpp, currentClass,members);
                }
                members.clear();
                currentClass.clear();
            }
        }
    }
    catch (FbyBaseExceptionPtr except)
    {
        cout << "Caught Error: " << except->Message << endl;
    }
    if (!currentClass.empty())
    {
        PrintSQLDefinition(ossql, oscpp, currentClass,members);
        members.clear();
    }
    fbcpp.close();
    fbsql.close();

}

int main(int argc, const char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        ParseFile(argv[i]);
    }
}
