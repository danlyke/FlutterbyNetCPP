#include "wiki.h"
#include <stdio.h>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <fstream>
#include <iostream>
#include <string.h>
#include <algorithm>

#include <sys/stat.h>
#include <sys/types.h>

#include "fbywikistrings.h"
#include "fbyfilefind.h"
#include "fbyregex.h"
#include "fbyregexparser.h"
#include "fbystring.h"
#include "wikiopenlayers.h"
#include "wikidpl.h"
#include "wikivideoflash.h"
#include "wikistatusupdate.h"
#include "fbyimageutils.h"
#include "fbyfileutils.h"

using namespace std;


using namespace std;
using namespace boost::iostreams ;
namespace fs = boost::filesystem;

string dotWiki(".wiki");
bool debug_output(0);

#include "wikidb.h"

class TreeBuilderWiki : public TreeBuilder {
    string google_maps_api_key;
    WikiDBPtr wikidb;
public:
    TreeBuilderWiki(string google_maps_api_key, WikiDBPtr wikidb)
        : google_maps_api_key(google_maps_api_key), wikidb(wikidb) {}
    ParseTreeNode *NodeFactory(const string &tagname);
};

ParseTreeNode *TreeBuilderWiki::NodeFactory(const string &tagname)
{
    if (tagname == "openlayers") return new OpenLayersNode();
    if (tagname == "DPL" || tagname == "dpl")
        return new DPLNode();
    if (tagname == "videoflash") return new VideoflashNode();
    if (tagname == "statusupdate") return new StatusUpdateNode(wikidb);
    return new ElementNode(tagname);
}




// "hostaddr = '127.0.0.1' port = '' dbname = 'fwaggle' user = 'fwaggle' password = 'password' connect_timeout = '10'"

Wiki::Wiki(FbyDBPtr db) : BaseObj(BASEOBJINIT(Wiki)),
                          wikidb(FBYNEW WikiDB(db)),
               staging_area(),
               output_area(),
               input_area(),
               google_maps_api_key()
{
}

string Wiki::WebPathFromFilename(const string &filename)
{
    string webpath(filename);
    if (boost::starts_with(webpath, output_area))
        webpath = webpath.substr(output_area.length() - 1);
    return webpath;
}


void Wiki::BeginTransaction()
{
    wikidb->BeginTransaction();
}

void Wiki::EndTransaction()
{
    wikidb->EndTransaction();
}



static bool StartsWithImage(const string &wikiname, string &imagename)
{
    if (wikiname.substr(0,6) == "Image:")
    {
        imagename = wikiname.substr(6);
        return true;
    }
    return false;
}

bool Wiki::Exists(const string & wikiname)
{
    WikiEntryPtr wikiTo;
    string imagename;
    if (StartsWithImage(wikiname, imagename))
    {
        ImagePtr image;

        if (wikidb->LoadImage(image, imagename))
            return true;
    }

    return wikidb->LoadWikiEntry(wikiTo, wikiname) && !(wikiTo->inputname.empty());
}

string Wiki::LoadFileToString(const char *filename)
{
    string fn(filename);
    string mainpage("Main Page.wiki");

    if (debug_output)
        cout << "Loading file '" << filename << "' to string" << endl;
    FILE *f = fopen(filename, "r");
    if (NULL == f && (fn.substr(0,input_area.length()) != input_area))
    {
        string fully_qualified_name = input_area + "/" + fn;
        f = fopen(fully_qualified_name.c_str(), "r");
        if (NULL == f)
        {
            string errmsg("Unable to open: ");
            errmsg += fully_qualified_name;
            cerr << errmsg << endl;
            THROWEXCEPTION(errmsg);
        }
    }

    if (NULL == f)
    {
        string errmsg("Unable to open: ");
        errmsg += filename;
        cerr << errmsg << endl;
        THROWEXCEPTION(errmsg);
    }
    fseek(f, 0L, SEEK_END);
    size_t length = ftell(f);
    fseek(f, 0L, SEEK_SET);
    
    char *buffer = new char[length + 1];
    buffer[length] = 0;
    fread(buffer, length, 1, f);
    fclose(f);
   
    length = RemoveCRs(buffer, length);
    string s(buffer, length);
    delete[] buffer;
    return s;
}

void Wiki::ScanWikiEntryForLinks(WikiEntryPtr wikiEntry)
{
    TreeBuilderWiki treeBuilder(google_maps_api_key, wikidb);

    string buffer = LoadWikiText(wikiEntry);
    {
        MarkedUpTextParser treeParser;
        treeParser.Parse(treeBuilder, buffer.c_str(),buffer.length());
    }

    vector<WikiEntryReferencePtr> wikiEntryReferences;
    wikidb->LoadWikiReferencesFrom(wikiEntryReferences, wikiEntry->wikiname);

    vector<string> currentLinks;
    for_each(wikiEntryReferences.begin(),
             wikiEntryReferences.end(),
             [&currentLinks](WikiEntryReferencePtr wikiEntryReference)
             { currentLinks.push_back(wikiEntryReference->to_wikiname); });

    vector<string> addLinks;
    vector<string> delLinks;
    HTMLOutputterOutboundLinks olinks;
    treeBuilder.AsHTML(olinks);

    for_each(currentLinks.begin(), currentLinks.end(),
             [&olinks, &delLinks](const string &wikiname)
             {
                 if (olinks.GetLinks().end() == find(olinks.GetLinks().begin(),
                                                     olinks.GetLinks().end(), wikiname))
                     delLinks.push_back(wikiname);
             });

    for_each(olinks.GetLinks().begin(), olinks.GetLinks().end(),
             [&currentLinks, &addLinks](const string &wikiname)
             {
                 if (currentLinks.end() == find(currentLinks.begin(),
                                                currentLinks.end(),
                                                wikiname))
                     addLinks.push_back(wikiname);
             });

    for_each(delLinks.begin(), delLinks.end(),
             [&](const string &wikiname)
             {
                 this->wikidb->DeleteWikiReference(wikiEntry->wikiname, wikiname);
             });

    for_each (addLinks.begin(), addLinks.end(),
             [&](const string &wikiname)
              {
                  this->wikidb->AddWikiReference(wikiEntry->wikiname, wikiname);
              });
}

void Wiki::ScanWikiFileForLinks(const char *filename)
{
    string thisWikiName(filename);
    size_t pos;
    pos = thisWikiName.rfind('/');
    if (pos != string::npos) thisWikiName.erase(0, pos + 1);
    pos = thisWikiName.rfind('.');
    if (pos != string::npos)
    {
        if (thisWikiName.substr(pos) != dotWiki)
            return;
        if (thisWikiName[0] == '.')
            return;

        thisWikiName.erase(pos);
    }
    thisWikiName = NormalizeWikiName(thisWikiName);

    WikiEntryPtr wikiEntry;

    if (debug_output)
        cout << "Should be creating '" << thisWikiName << "'" << endl;

    if (wikidb->LoadOrCreateWikiEntry(wikiEntry, thisWikiName)
        || wikiEntry->inputname.empty())
    {
        if (debug_output)
            cout << "Marking '" << thisWikiName << "' dirty, wasLoaded is " << wikiEntry->WasLoaded() << endl;

        wikiEntry->inputname = filename;
        wikiEntry->needsContentRebuild = true;
        wikidb->WriteWikiEntry(wikiEntry);
    }

    ScanWikiEntryForLinks(wikiEntry);
    
}


string Wiki::ImageNameFromImageFileName( const string & filename )
{
    string wikiname(filename);
    size_t pos = wikiname.find('!');
    if (pos != string::npos)
        wikiname = wikiname.erase(0, pos + 1);

    pos = wikiname.find("px-");
    if (pos != string::npos)
    {
        size_t i;
        for (i = 0; i < pos; ++i)
        {
            if (!isdigit(wikiname[i]))
                break;
        }
        wikiname = wikiname.erase(0,pos+3);
    }
    string imagename = wikiname;
    return imagename;
}

void Wiki::LoadJPEGData(const std::string &imagepath)
{
    std::string path(imagepath);
    time_t lastWriteTime(last_write_time(imagepath));
    ImageInstancePtr imageInstance;

    if (debug_output)
        cout << "LoadJPEGData: Looking for " << imagepath << endl;

    if (wikidb->LoadImageInstance(imageInstance, imagepath))
    {
        if (debug_output)
            cout << "Comparing " << lastWriteTime << " to " << imageInstance->mtime << endl;

        if (lastWriteTime <= imageInstance->mtime)
        {
            if (debug_output)
                cout << "  last write time " << lastWriteTime << " was less than "
                     << imageInstance->mtime << endl;

            return;
        }
    }

    string filename(path);
    size_t pos = filename.rfind("/");
    if (pos != string::npos)
    {
        filename = filename.erase(0, pos + 1);
    }

    pos = filename.rfind('.');
    if (pos != string::npos
        && !strcasecmp(filename.c_str() + pos, ".jpg"))
    {
        string imagename(ImageNameFromImageFileName(filename));
        string wikiname("Image:" + imagename);

        int width = -1, height = -1;
        map<string,string> attributes;
        if (FindJPEGSize(imagepath, width, height, attributes))
        {
            if (debug_output)
                cout << "Found size " << width << " by " << height << " for " << imagename << " path " << imagepath << endl;

            ImagePtr image;
            wikidb->LoadOrCreateImage(image, imagename);

            WikiEntryPtr wikientry;
            bool writeWikiEntry = false;

            if (wikidb->LoadOrCreateWikiEntry(wikientry, wikiname))
            {
                if (debug_output)
                    cout << "LoadOrCreate wikientry returned true\n";

                wikientry->needsContentRebuild = true;
                writeWikiEntry = true;
            }

            if (wikidb->LoadOrCreateImageInstance(imageInstance, imagepath))
            {
                if (debug_output)
                    cout << "LoadOrCreate imageinstance returned true\n";

                imageInstance->width = width;
                imageInstance->height = height;
                imageInstance->mtime = lastWriteTime;
                imageInstance->imagename = imagename;

                wikidb->WriteImageInstance(imageInstance);

                wikientry->needsContentRebuild = true;
                writeWikiEntry = true;
            }
            if (writeWikiEntry)
            {
                wikidb->WriteWikiEntry(wikientry);
            }
            
        }
    }

}

void Wiki::LoadPNGData(const std::string &imagepath)
{
    std::string path(imagepath);

//    cout << "Path is " << path << endl;
    string filename(path);
    size_t pos = filename.rfind("/");
    if (pos != string::npos)
    {
        filename = filename.erase(0, pos + 1);
    }

    pos = filename.rfind('.');
    if (pos != string::npos
        && !strcasecmp(filename.c_str() + pos, ".jpg"))
    {
        string wikiname(filename);
//        int width = -1, height = -1;
//        cout << "Wiki name Image:" << wikiname << endl;
//        FindPNGSize(filename, width, height);
    }
}




using namespace std;
namespace fs = boost::filesystem;
fs::directory_iterator end_iter;

//                 # .stem!




class CompareWithStaging
{
public:
    string_and_time_map_t &rs;
    CompareWithStaging(string_and_time_map_t &_rs) : rs(_rs) {};
    void operator ()(fs::directory_iterator dir_iter)
    {
        string wikiname(NormalizeWikiName(dir_iter->path().filename().stem().string()));
        string filename(NormalizeWikiNameToFilename(wikiname));
        string destname(filename + ".html");
        string_and_time_map_t::iterator source = rs.find(destname);
                
        if (rs.end() != source)
        {
            std::time_t t(fs::last_write_time(*dir_iter));
            if (debug_output)
                cout << wikiname << " as " << filename << " comparing " << t << " to " << source->second << endl;

        }
    }
};




void Wiki::ScanWikiFiles(const char *inputDir, const char *stagingDir)
{
    string_and_time_map_t stagingFiles;

    FindFileNames(stagingFiles, stagingDir, ".html");

    FileFind(inputDir, [this, &stagingFiles](fs::directory_iterator dir_iter)
             {
                 if (dir_iter->path().filename().extension() == dotWiki)
                 {
                     string inputname(dir_iter->path().filename().string());
                     if (inputname[0] == '.')
                         return;

                     string basename(dir_iter->path().filename().stem().string());
                     string wikiname(NormalizeWikiName(basename));
                     string filename(NormalizeWikiNameToFilename(wikiname));
                     string destname(filename + ".html");
                     string_and_time_map_t::const_iterator source = stagingFiles.find(destname);

                     WikiEntryPtr entry;
                     if (!wikidb->LoadOrCreateWikiEntry(entry, wikiname))
                     {
                         bool dirty = false;
                         if (entry->inputname.empty())
                         {
                             entry->inputname = inputname;
                             dirty = true;
                         }

                         if (stagingFiles.end() != source)
                         {
                             std::time_t t(fs::last_write_time(*dir_iter));
                             if (t > source->second)
                                 dirty = true;
                         }
                         else
                         {
                             dirty = true;
                             
                         }
                         if (dirty)
                         {
                             cout << "Marking " << wikiname
                                  << " dirty, would write to " << destname;
                             cout << "   " << (stagingFiles.end() != source ? "time" : "missing dest") << endl;
                             cout << endl;

                             entry->needsContentRebuild = true;
                             wikidb->WriteWikiEntry(entry);
                         }
                     }
                     else
                     {
                         if (debug_output)
                             cout << "Adding " << wikiname << endl;

//                         entry->wikiname = wikiname;
                         entry->inputname = inputname;
                         entry->needsContentRebuild = true;
                         entry->needsExternalRebuild = false;
                         wikidb->WriteWikiEntry(entry);
                     }
                 }
             });
//    FileFind("/home/danlyke/websites/flutterby.net/mvs", CompareWithStaging(stagingFiles));
}


void Wiki::ScanWikiFilesForLinks(const char *inputDir, const char * /* stagingDir */)
{
    string_and_time_map_t stagingFiles;

    FileFind(inputDir,
             [this](fs::directory_iterator dir_iter)
             {
                 string path(dir_iter->path().string());
                 ScanWikiFileForLinks(path.c_str());
             }
        );
}

string Wiki::LoadWikiText(WikiEntryPtr wikiEntry)
{
    string fileContents;
    if (debug_output)
        cout << "Attempting to load file text for '" << wikiEntry->wikiname << "'" << endl;

    string imagename;
    if (StartsWithImage(wikiEntry->wikiname, imagename) && wikiEntry->inputname.empty())
    {
        ImagePtr image;

        if (wikidb->LoadImage(image, imagename))
        {
            if (wikidb->ImageHasInstances(image))
            {
                ImageInstancePtr fullsize(wikidb->ImageInstanceFullsize(image));
                ImageInstancePtr thumb(wikidb->ImageInstanceThumb(image));
                ImageInstancePtr original(wikidb->ImageInstanceOriginal(image));
                
                fileContents += "== ";
                fileContents += ConvertImageNameToDescription(imagename);
                fileContents += " ==\n\n";
                fileContents += "<img src=\"" + WebPathFromFilename(fullsize->filename)
                    + "\" width=\"" + to_string(fullsize->width)
                    + "\" height=\"" + to_string(fullsize->height)
                    + "\" >\n\n";

                vector<ImageInstancePtr> instances = wikidb->ImageInstances(image);

                fileContents += "All sizes: ";
                for (auto instance = instances.begin();
                     instance != instances.end();
                     ++instance)
                {
                    if (instance != instances.begin())
                        fileContents += " | ";

                    fileContents +=  "<a href=\"" + WebPathFromFilename((*instance)->filename)
                        + "\">" + to_string((*instance)->width)
                        + "x" + to_string((*instance)->height)
                    + "</a>";

                }
                fileContents += "\n\n";


                int width(0), height(0);
                map<string,string> attributes;
                if (FindJPEGSize(original->filename, width, height, attributes))
                {
                    fileContents += "<ul>";
                    for (auto attr = attributes.begin(); attr != attributes.end(); ++attr)
                    {
                        fileContents += "<li><em>" + attr->first + "</em> : " + attr->second + "</li>\n";
                    }
                    fileContents += "</ul>\n";
                }

                vector<StatusUpdatePtr> statuses;
                wikidb->LoadStatusUpdatesForImage(statuses, imagename);
                for (auto status = statuses.begin();
                     status != statuses.end();
                     ++status)
                {
                    fileContents += "\n\n" + (*status)->status + "\n\n";
                }
            }
        }
    }
    else
    {
        string filename(wikiEntry->inputname);
        if (!filename.empty())
            fileContents = LoadFileToString(filename.c_str());
    }
    return fileContents;
}



void Wiki::DoWikiFile(string wikiname)
{
    WikiEntryPtr wikientry;
    if (wikidb->LoadWikiEntry(wikientry, wikiname))
    {
        string s(LoadWikiText(wikientry));

        if (!s.empty())
        {
            ParseWikiBufferToOutput(wikientry->wikiname,
                                    s.data(), s.length(),
                                    staging_area.c_str());
            
            wikientry->needsContentRebuild = false;
            wikientry->needsExternalRebuild = false;
            wikidb->WriteWikiEntry(wikientry);
        }
    }
    else
    {
        cerr << "Wiki entry '" << wikiname << "' not found" << endl;
    }
    CopyChangedFiles(staging_area, output_area);
}


void Wiki::GetContentDirty()
{
    vector<WikiEntryPtr> wikientries;
    wikidb->LoadContentDirtyWikiEntries(wikientries);
    for(auto wikientry = wikientries.begin();
        wikientry != wikientries.end();
        ++wikientry)
    {
        cout << (*wikientry)->wikiname << endl;
    }
}

void Wiki::GetReferencedDirty()
{
    vector<WikiEntryPtr> wikientries;
    wikidb->LoadReferencedDirtyWikiEntries(wikientries);
    for(auto wikientry = wikientries.begin();
        wikientry != wikientries.end();
        ++wikientry)
    {
        cout << (*wikientry)->wikiname << endl;
    }
}


void Wiki::RebuildDirtyFiles(const char *outputdir)
{
    vector<WikiEntryPtr> wikientries;
    wikidb->LoadDirtyWikiEntries(wikientries);
    for(auto wikientry = wikientries.begin();
        wikientry != wikientries.end();
        ++wikientry)
    {
        if (debug_output)
            cout << "Rebuilding " << (*wikientry)->wikiname << endl;

        string s(LoadWikiText(*wikientry));

        if (!s.empty())
        {
            ParseWikiBufferToOutput((*wikientry)->wikiname,
                                    s.data(), s.length(),
                                    outputdir);
        }

        if (debug_output)
            cout << "Marking '" << (*wikientry)->wikiname << "' clean" << endl;
        (*wikientry)->needsContentRebuild = false;
        (*wikientry)->needsExternalRebuild = false;
        wikidb->WriteWikiEntry(*wikientry);
    }
}

void Wiki::ParseAndOutputFile(const char *inputfile, const char *outputdir)
{
    string buffer = LoadFileToString(inputfile);
    
    string wikiname(FilenameToWikiName(string(inputfile)));
    ParseWikiBufferToOutput(wikiname, buffer.c_str(), buffer.length(), outputdir);
}

void Wiki::ParseWikiBufferToOutput(string wikiname, const char *buffer, size_t length, const char * outputdir)
{
    string outputpath(outputdir);
    bool includeLoginManager(true);
    TreeBuilderWiki treeBuilder(google_maps_api_key, wikidb);
    {
        MarkedUpTextParser treeParser;
        treeParser.Parse(treeBuilder, buffer,length);
    }


    if (debug_output)
        cerr << "Parsing wiki buffer to output '" << wikiname << "' outputdir '" << outputdir << "'" << endl;


    outputpath += "/" + NormalizeWikiNameToFilename(wikiname) + ".html";
    
    if (debug_output)
        cout << "Opening " << outputpath << endl;

    std::filebuf output;
    output.open(outputpath, std::ios::out);
    ostream os(&output);
    string onload;

    os << "<!DOCTYPE html>\n";
    os << "<html><head><title>" << wikiname << "</title>\n";
    os << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" >\n";
    os << "<style type=\"text/css\">@import \"/screen.css\";</style>\n";
    os << "<link href=\"/favicon.ico\" rel=\"icon\" type=\"image/ico\">\n";
    os << "<link href=\"/favicon.ico\" rel=\"shortcut icon\">\n";

    if (treeBuilder.HasA("openlayers")
        || treeBuilder.ReferencesImage()
        || includeLoginManager)
    {
        os << "<script type=\"text/javascript\" src=\"js/jquery-1.11.0.min.js\"></script>\n";
    }

    if (includeLoginManager)
    {
        string login_manager("/cgi-bin/loginmgr.pl");
        os << "<script src=\"js/jquery.cookie.js\" type=\"text/javascript\" ></script>\n";

        onload += "if ($.cookie('magic')) { $.get('"
            + login_manager + "', { n:'"
            + wikiname + "' }, function(data,status,request) { $('#login').html(data); }); }\n";
    }
    
    if (treeBuilder.ReferencesImage())
    {
        os << "<script type=\"text/javascript\" src=\"js/lightbox.min.js\"></script>\n";
        os << "<link rel=\"stylesheet\" href=\"css/lightbox.css\" type=\"text/css\" media=\"screen\">\n";
//        os << "<script type=\"text/javascript\">"
//            "$(function() {\n"
//            "    // Use this example, or...\n"
//            "    $('a[rel|=\"lightbox\"]').lightBox(); // Select all links that contains lightbox in the attribute rel\n"
//            "});\n"
//            "</script>\n";
    }
    if (treeBuilder.HasA("googlemap") || treeBuilder.HasA("openlayers")) {
        os << "<script type=\"text/javascript\" src=\"http://maps.google.com/maps/api/js?sensor=false&v=3.6&key=" << google_maps_api_key << "\"></script>\n";
    }
    if (treeBuilder.HasA("openlayers"))
    {
        os << "<script type=\"text/javascript\" src=\"js/fby/maputils.js\"></script>\n";
    }
    if (treeBuilder.HasA("googlemap") || treeBuilder.HasA("openlayers")) {
        os << "<script type=\"text/javascript\">"
            "//<![CDATA[\n"
            "var mapIcons = {};function addLoadEvent(func) {var oldonload = window.onload;if (typeof oldonload == 'function') {window.onload= function() {oldonload();func();};} else {window.onload = func;}}\n"
            "//]]>\n</script>\n";
    }
    if (treeBuilder.HasA("openlayers")) {
        os <<
            "<link rel=\"stylesheet\" href=\"js/ol/default/style.css\" type=\"text/css\">\n"
            "\n"
            "<link  rel=\"stylesheet\" href=\"js/ol/default/google.css\" type=\"text/css\">\n"
            "\n"
            "<script type=\"text/javascript\" src=\"js/OpenLayers-2.11/OpenLayers.js\">\n"
            "</script>\n"
            "<script type=\"text/javascript\" src=\"js/OSM_LocalTileProxy.js\">\n"
            "</script>\n";
    }


    if (!onload.empty())
    {
        os << "<script type=\"text/javascript\">";
        os << "//<![CDATA[\n";
        os << "$(document).ready(function() {\n";
        os << onload;
        os << "});\n";
        os << "//]]>\n";
        os << "</script>\n";
    }

    if (wikiname == "User:DanLyke")
    {
        os << "<link rel=\"openid2.provider\" href=\"https://www.google.com/accounts/o8/ud\">\n";
        os << "\n";
        os << "<link rel=\"openid2.local_id\" href=\"https://profiles.google.com/danlyke1\">\n";
        os << "<link rel=\"meta\" type=\"application/rdf+xml\" title=\"FOAF\" href=\"http://www.flutterby.net/User%3aDanLyke_foaf.rdf\" >\n";
        os << "<link rel=\"alternate\" type=\"text/directory\"\n";
    }
    if (wikiname.substr(0,9) == "Category:")
    {
        os << "<link rel=\"alternate\" href=\"http://www.flutterby.net/$syndifile.rss\" title=\"RSS\" type=\"application/rss+xml\" >\n";
    }

    os << "</head>\n";
    os << "<body>\n";
    if (includeLoginManager)
    {
        os << "<div id=\"login\"></div>";
    }
    os << "<h1>" << wikiname << "</h1>\n";

    os << "<div class=\"sidebar\"><div class=\"navbar\"><h2>Navigation</h2>\n";
    os << "<ul><li><a href=\"Main_Page\">Main Page</a>\n";
    os << "</li><li><a href=\"Categories\">Categories</a>\n";
    os << "</li><li><a href=\"User%3aDanLyke\">Dan Lyke</a>\n";
    os << "</li></ul>\n";
    os << "</div>";

    vector<WikiEntryReferencePtr> wikiEntryReferences;
    wikidb->LoadWikiReferencesTo(wikiEntryReferences, wikiname);

    if (!wikiEntryReferences.empty())
	{
        os << "<div class=\"linkshere\">\n<h2>Pages which link here</h2>\n<ul>";

        for_each(wikiEntryReferences.begin(),
                 wikiEntryReferences.end(),
                 [&os] (WikiEntryReferencePtr wikiEntryReference)
                 {
                     os << "<li><a href=\"./";
                     os << NormalizeWikiNameToFilename(wikiEntryReference->from_wikiname);
                     os << "\">";
                     os << wikiEntryReference->from_wikiname;
                     os << "</a></li>\n";
                 }
            );
        os << "</ul></div>\n";
	}

    os << "</div><div class=\"contentcolumn\">";

    if (wikiname == "User:DanLyke")
    {
        os << "<div class=\"precontent\"><p class=\"h-card vcard\">\n";
        os << "  <img class=\"u-photo\" width=\"154\" height=\"160\" align=\"right\" src=\"http://www.flutterby.net/files/images/14/09/22/640px-BoatBuildingPicAsPortrait.jpg\" alt=\"\" >\n";
        os << "  <a class=\"p-name u-url url fn\" href=\"http://www.flutterby.net/User:DanLyke\">Dan Lyke</a><br >\n";
        os << "  <a class=\"u-email email\" href=\"mailto:danlyke@flutterby.com\">danlyke@flutterby.com</a>,<br > \n";
        os << "  <span class=\"p-tel tel\">415-342-5180</span> (cell)<br >\n";
        os << "  <span class=\"p-street-address street-address\">10 Mission Drive</span>,<br >\n";
        os << "  <span class=\"p-locality locality\">Petaluma</span>\n";
        os << "  <span class=\"p-region region\">California\n";
        os << "  <span class=\"p-postal-code postal-code\">94952</span>\n";
        os << "  <span class=\"p-country-name country-name \">US</span>\n";
        os << "  <br clear=\"both\" >\n";
        os << "</p></div>\n";
    }

    vector<ParseTreeNodePtr> sections;
    treeBuilder.ForEach(
        [&sections](ParseTreeNodePtr n)
        {
            if (n->Name().length() == 2
                && n->Name()[0] == 'h'
                && isdigit(n->Name()[1]))
            {
                sections.push_back(n);
            }
        });
    if (sections.size() > 1)
    {
        ParseTreeNodePtr div(new ElementNode("div"));
        div->AddAttribute("class");
        div->AddAttributeValue("sections");
        
        ParseTreeNodePtr h2(new ElementNode("h2"));
        h2->AddText("Sections");
        div->AddChild(h2);

        ParseTreeNodePtr ul(new ElementNode("ul"));
        div->AddChild(ul);

        for (auto section = sections.begin(); section != sections.end(); ++section)
        {
            string section_name;

            (*section)->ForEachChild([&section_name](ParseTreeNode *node)
                                     {
                                         if (node->Name() == "a"
                                             && !node->GetAttribute("name").empty())
                                         {
                                             section_name += node->GetAttribute("name");
                                         }
                                     });
            ParseTreeNodePtr a(new ElementNode("a"));
            
            a->AddAttribute("href");
            a->AddAttributeValue("#");
            a->AddAttributeValue(section_name);
            a->AddText((*section)->GetText());

            ParseTreeNodePtr li(new ElementNode("li"));
            li->AddChild(a);
            ul->AddChild(li);
        }

        treeBuilder.Graft(div, 1);
    }

/*
        if ($i == 1)
        {
            unshift @contentdiv,
                (
                 'div',
                 [ {class=>'sections'}, 
                   'h2', [{}, '0', 'Sections' ],
                   '0', "\n",
                   @list,
                   '0', "\n",
                 ],
                 );
        }
        elsif ($i < @$subbody)
        {
            my @rest = splice @$subbody, $i, scalar($subbody) - $i;
            push @contentdiv,
                (
                 'div',
                 [ {class=>'sections'}, 
                   'h2', [{}, '0', 'Sections' ],
                   '0', "\n",
                   @list,
                   '0', "\n",
                 ],
                 'div',
                 [ {class => 'content'}, @rest],

                );
        }
*/

    HTMLOutputterWikiString outputter(os, WikiPtr(this), wikiname);
    
    treeBuilder.AsHTML(outputter);
os << "<br clear=\"all\"></br><div class=\"footer\">";
if (treeBuilder.HasA("openlayers"))
{
    os << "<p>Gratefully acknowledged: "
        "<a href=\"http://www.openstreetmap.org/\">OpenStreetMap</a> "
        "map tiles &copy; OpenStreetMap contributors, cartography is licensed under the "
        "<a href=\"http://creativecommons.org/licenses/by-sa/2.0/\"> "
        "Creative Commons Attribution-ShareAlike 2.0 license"
        "</a>"
        " and tiles are cached on my servers (so may be out of date). "
        "<a href=\"http://maps.google.com/\">Google Maps</a>"
        " map tiles are used under the terms of their license.";
}
os <<   "<p>Flutterby.net is a publication of "
        "<a href=\"mailto:danlyke@flutterby.com\">Dan Lyke</a> "
        "and unless otherwise noted, copyright by Dan Lyke"
        "</p></div></div></body></html>";
    os << endl;
}



void Wiki::GetImageHTML(ostream &os,
                        const string &thisWikiName,
                        const string &wikiname,
                        const string &imagename,
                        const string &text)
{
    ImagePtr img;

    if (debug_output)
        cout << "Searching for image " << wikiname << " " << imagename << endl;


    bool hasImage = wikidb->LoadImage(img, imagename);
    vector<string> notes(split('|', text));
    string align;
    string desc;
    string width;
    string spanclass("image");
    string caption;
    vector<ImageInstancePtr> imginstances;

    for (auto note = notes.begin(); note != notes.end(); ++note)
    {
        if (*note == "thumb" && hasImage)
        {
            if (wikidb->ImageHasInstances(img))
            {
                ImageInstancePtr instance(wikidb->ImageInstanceThumb(img));
                if (FBYTYPEDNULL(ImageInstancePtr) != instance)
                {
                    imginstances.push_back(instance);
                }
            }
        }
        else if (*note == "full" && hasImage)
        {
            if (wikidb->ImageHasInstances(img))
            {
                ImageInstancePtr instance(wikidb->ImageInstanceFullsize(img));
                if (FBYTYPEDNULL(ImageInstancePtr) != instance)
                {
                    if (debug_output)
                        cout << "Found full " << instance->filename << endl;

                    imginstances.push_back(instance);
                }
            }
        }
        else if (*note == "left" || *note == "right")
        {
            align = *note;
        }
        else if (*note == "frame")
        {
            spanclass = "imageframed";
        }
        else if (*note == "none")
        {
        }
        else
        {
            desc += *note;
        }
    }

    if (imginstances.empty())
    {
        if (hasImage && wikidb->ImageHasInstances(img))
        {
            ImageInstancePtr instance(wikidb->ImageInstanceFullsize(img));
            if (FBYTYPEDNULL(ImageInstancePtr) != instance)
            {
                if (debug_output)
                    cout << "Defaulted " << instance->filename << endl;

                imginstances.push_back(instance);
            }
        }
    }
    for (auto imginst = imginstances.begin();
         imginst != imginstances.end();
         ++imginst)
    {
        if (ImageInstancePtr() != *imginst)
        {
            if ((*imginst)->NeedsDimensionsLoaded())
            {
                (*imginst)->LoadDimensions();
                wikidb->WriteImageInstance(*imginst);
            }
            if ((*imginst)->width > 0)
            {
                width = "style=\"width: "
                    + to_string((*imginst)->width) + "px;\"";
            }
            if (imginstances.size() > 1)
            {
                caption += " (" + to_string((*imginst)->width)
                    + " by " + to_string((*imginst)->height) + ")";
            }
        }
        if (!hasImage)
        {
            spanclass = "imagemissing";
//            cerr <<  "Missing " << imagename << " from database\n";
        }
        
        os <<  "<span class=\"" << spanclass << align
           << "\" " << width << ">";
        WikiEntryPtr wikientry;
        if (wikidb->LoadOrCreateWikiEntry(wikientry, wikiname))
            wikidb->WriteWikiEntry(wikientry);

        wikidb->AddWikiReference(thisWikiName, wikiname);


        bool targetExists = Exists(wikiname);
        
        if (targetExists)
            os << "<a href=\"./" << wikiname << "\">";
        if (ImageInstancePtr() != *imginst)
        {
            os << "<img src=\"" << WebPathFromFilename((*imginst)->filename)
               << "\" width=\"" << to_string((*imginst)->width)
               << "\" height=\"" << to_string((*imginst)->height)
               << "\" alt=\"" << "\" >";
        }
        else
        {
            os << "<em>Image not found: " << wikiname << "</em>" << endl;
        }
        

        if (targetExists)
            os <<  "</a>";

        if (hasImage)
        {
            ImageInstancePtr zoominst(wikidb->ImageInstanceLightboxZoom(img));
            if (ImageInstancePtr() != zoominst)
            {
                os << "<a href=\"./";
                os << WebPathFromFilename(zoominst->filename);
                os << "\" data-lightbox=\"screen\" data-title=\"";
                os << caption;
                os << "\">";
                os << "<span class=\"imagezoombox\">&nbsp;</span></a>\n";
            }
        }
        if (!caption.empty())
        {
            os << "<span class=\"imagecaption\"><p class=\"imagecaption\">";
            os << caption;
            os << "</p></span>";
        }
        os << "</span>";
    }
}

HTMLOutputterString::HTMLOutputterString(ostream &os)
    : HTMLOutputter(),
      os(os)
{
}

void HTMLOutputterString::AddString(const string &s)
{
    os << s;
}


bool EndTagShouldBeSuppressed(const string &name)
{
  return (name == "img") || (name == "br");
}

void HTMLOutputterString::AddHTMLNodeBegin(const string &name,
                                           const vector< pair<string,string> > & attributes,
                                           bool empty_node)
{
    os <<  "<" << name;
    if (!attributes.empty())
    {
        for (auto attr = attributes.begin();
             attr != attributes.end(); ++ attr)
        {
            os <<  " " << attr->first << "=\"" << attr->second << "\"";
        }
    }
    //   if (empty_node)
    // os <<  "/";
    os <<  ">";
}

void HTMLOutputterString::AddHTMLNodeEnd(const string &name)
{
    if (!EndTagShouldBeSuppressed(name))
        os <<  "</" << name << ">";
    if (name == "p") os << endl << endl;
    if (name == "li") os << endl;
    if (name == "ul") os << endl;
    if (name == "ol") os << endl;
    if (name == "blockquote") os << endl;
}
void HTMLOutputterString::AddWikiLink(const string &wikiname,
                                      const string &text)
{
    os <<  "<a href=\"./" << wikiname << "\">" << text << "</a>";
}

void Wiki::LoadDPLEntries(vector<WikiEntryPtr> & entries,
                          const string &category,
                          const string &order,
                          const string &pattern,
                          const string &count)
{
    wikidb->LoadDPLEntries(entries, category, order, pattern, count);
}


void HTMLOutputterWikiString::AddDPLList(const string &category,
                                         const string &order,
                                         const string &pattern,
                                         const string &count)
{
    vector<WikiEntryPtr> entries;

    wiki->LoadDPLEntries(entries, category, order, pattern, count);

    if (!entries.empty())
    {
        AddString("<ul>");
        for (auto wikientry = entries.begin(); wikientry != entries.end(); ++wikientry)
        {
            AddString("<li><a href=\"./");
            AddString(NormalizeWikiNameToFilename((*wikientry)->wikiname));
            AddString("\">");
            AddString((*wikientry)->wikiname);
            AddString("</a></li>\n");
        }
        AddString("</ul>");
    }
}
                                      

void HTMLOutputterWikiString::AddWikiLink(const string &wikiname,
                                          const string &text)
{
    const string name(NormalizeWikiName(wikiname));
    const string linktarget(NormalizeWikiNameToFilename(name));

    string imagename;

    if (StartsWithImage(wikiname, imagename))
    {
        wiki->GetImageHTML(os, thisWikiName, wikiname, imagename, text);
    }
    else
    {
        if (debug_output)
            cout << "Searching for wiki link " << name << endl;;

        if (wiki->Exists(name))
        {
            os <<  "<a href=\"./" << linktarget << "\">" << text << "</a>";
        }
        else
        {
            os << "<cite>" << text << "</cite>";
        }
    }
}

HTMLOutputterString::~HTMLOutputterString()
{
}


HTMLOutputterOutboundLinks::HTMLOutputterOutboundLinks()
    : HTMLOutputter(), wikiLinks()
{
}

void HTMLOutputterOutboundLinks::AddWikiLink(const string &wikiname, const string & /* text */)
{
    if (std::find(wikiLinks.begin(), wikiLinks.end(), wikiname) == wikiLinks.end())
    {
        wikiLinks.push_back(wikiname);
    }

}






int Wiki::ScanImages(const char *path)
{
    int imagecount;
    if (debug_output)
        cout << "Searching " << path << endl;

    FileFind(path,
             [this, &imagecount](fs::directory_iterator dir_iter)
             {
                 if (!strcasecmp(dir_iter->path().filename().extension().c_str(), ".jpg"))
                 {
                     ++imagecount;
                     this->LoadJPEGData(dir_iter->path().c_str());
                 }
                 else if (!strcasecmp(dir_iter->path().filename().extension().c_str(), ".png"))
                 {
                     ++imagecount;
                     this->LoadPNGData(dir_iter->path().c_str());
                 }
             }
        );
    return imagecount;
}






// const char *staging_area = "/home/danlyke/websites/flutterby.net/html_staging/";
// const char *output_area = "/home/danlyke/websites/flutterby.net/public_html_static/";
// const char *input_area = "/home/danlyke/websites/flutterby.net/code/mvs/";


void Wiki::SetInputDirectory(string input_directory)
{
    input_area = input_directory;
}

void Wiki::SetStagingDirectory(string staging_directory)
{
    staging_area = staging_directory;
}

void Wiki::SetOutputDirectory(string output_directory)
{
    output_area = output_directory;
}

void Wiki::SetGoogleMapsAPIKey(string google_maps_api_key)
{
    this->google_maps_api_key = google_maps_api_key;
}


void Wiki::DoEverything()
{
    wikidb->NukeDatabase();
    wikidb->BeginTransaction();
    ScanImages(output_area);
    ScanWikiFilesForLinks(input_area, staging_area);
    ScanWikiFiles(input_area, staging_area);
    RebuildDirtyFiles(staging_area);
    wikidb->EndTransaction();
    CopyChangedFiles(staging_area, output_area);
}

void Wiki::RebuildDirtyFiles()
{
    RebuildDirtyFiles(staging_area);
    CopyChangedFiles(staging_area, output_area);
}


void Wiki::ScanWikiFiles()
{
    wikidb->BeginTransaction();
    ScanWikiFilesForLinks(input_area, staging_area);
    wikidb->EndTransaction();
}

void Wiki::ScanWikiFiles_NOCHANGES()
{
    wikidb->BeginTransaction();
    ScanWikiFiles(input_area, staging_area);
    wikidb->EndTransaction();
}

void Wiki::DoWikiFiles()
{
    wikidb->BeginTransaction();
//    ScanWikiFilesForLinks(input_area, staging_area);
    ScanWikiFiles(input_area, staging_area);
    CopyChangedFiles(staging_area, output_area);
    wikidb->EndTransaction();
}
void Wiki::GetWikiFiles()
{
    FileFind(input_area,
             [](fs::directory_iterator dir_iter)
             {
                 if (dir_iter->path().filename().extension() == dotWiki)
                 {
                     cout << dir_iter->path().stem().string() << endl;
                 }
             });
}

void Wiki::WriteWikiFile(string target_file)
{
    string outputpath(input_area);
    string wikiname(NormalizeWikiName(target_file));
    outputpath +=  wikiname + dotWiki;
    unsigned char buffer[1024];
    FILE *outputfile = fopen(outputpath.c_str(), "w");
    if (!outputfile)
        return;

    size_t len;

    while (0 != (len = fread(buffer, 1, sizeof(buffer), stdin)))
    {
        fwrite(buffer, 1, len, outputfile);
    }
    fclose(outputfile);
    

    WikiEntryPtr entry;
    if (wikidb->LoadWikiEntry(entry, wikiname))
    {
        entry->needsContentRebuild = true;
        wikidb->WriteWikiEntry(entry);
    }
    else
    {
        entry = WikiEntryPtr(FBYNEW(WikiEntry));
        entry->wikiname = wikiname;
        entry->inputname = outputpath;
        entry->needsContentRebuild = true;
        entry->needsExternalRebuild = false;
        wikidb->WriteWikiEntry(entry);
    }
    DoDirtyFiles();
}
Regex regexStartsWithYYYYMMDD("^(\\d{4})\\-(\\d{2})\\-(\\d{2})");

void Wiki::WriteImageFile(string target_file) 
{
    string imagefile(target_file);
    string localdir;
    string targetdir(output_area);
    targetdir += "files/images/";

    RegexMatch match;
    if (regexStartsWithYYYYMMDD.Match(imagefile, match))
    {
        targetdir += match.Match(1);
        mkdir(targetdir.c_str(), 0755);
        targetdir += "/" + match.Match(2);
        mkdir(targetdir.c_str(), 0755);
        targetdir += "/" + match.Match(3);
        mkdir(targetdir.c_str(), 0755);
    }
    else
    {
        time_t now(time(nullptr));
        struct tm *lt = localtime(&now);
        char timebuf[32];

        strftime(timebuf, sizeof(timebuf), "%y", lt);
        targetdir += timebuf;
        mkdir(targetdir.c_str(), 0755);
        strftime(timebuf, sizeof(timebuf), "/%m", lt);
        targetdir += timebuf;
        mkdir(targetdir.c_str(), 0755);
        strftime(timebuf, sizeof(timebuf), "/%d", lt);
        targetdir += timebuf;
        mkdir(targetdir.c_str(), 0755);
    }
    targetdir += "/";
    string targetfile(targetdir + imagefile);

    FILE *target = fopen(targetfile.c_str(), "wb");
    size_t buflen = 16384;
    unsigned char * buffer = new unsigned char[buflen];
    size_t len;
    while (0 != (len = fread(buffer, 1, buflen, stdin)))
    {
        fwrite(buffer, 1, len, target);
    }
    delete[] buffer;
    fclose(target);

    LoadJPEGData(targetfile);
    LoadJPEGData(CreateThumbnail(targetdir, imagefile, 160));
    LoadJPEGData(CreateThumbnail(targetdir, imagefile, 640));
    LoadJPEGData(CreateThumbnail(targetdir, imagefile, 1024));

    string upload_wiki("Status Uploaded Images");
    string upload_references(input_area + "/" + upload_wiki + ".wiki");
    FILE *uploaded = fopen(upload_references.c_str(), "a");
    string wikiname("Image:" + ImageNameFromImageFileName(imagefile));
    fprintf(uploaded, "[[%s]]\n", wikiname.c_str());
    fclose(uploaded);
    DoWikiFile(wikiname);
    DoWikiFile(upload_wiki);
}


void Wiki::ReadWikiFile(string target_file) 
{
    string wikifile(target_file);
    string filename(input_area + wikifile + dotWiki);
    FILE *target = fopen(filename.c_str(), "rb");
    size_t buflen = 16384;
    unsigned char * buffer = new unsigned char[buflen];
    size_t len;
    while (0 != (len = fread(buffer, 1, buflen, target)))
    {
        fwrite(buffer, 1, len, stdout);
    }
    delete[] buffer;
    fclose(target);
}

void Wiki::RebuildDetached()
{
    DoEverything();
}

void Wiki::ImportKML(string /* target_file */) { assert(0); }

void Wiki::DoDirtyFiles()
{
    RebuildOutputFiles();
    CopyChangedFiles(staging_area, output_area);
}

void Wiki::DoChangedFiles()
{
    wikidb->BeginTransaction();
    ScanWikiFiles(input_area, staging_area);
    RebuildDirtyFiles(staging_area);
    wikidb->EndTransaction();
    CopyChangedFiles(staging_area, output_area);
}

void Wiki::ScanImages()
{
    wikidb->BeginTransaction();
    ScanImages(output_area);
    wikidb->EndTransaction();
}

void Wiki::VerifyWikiLink( string target_file )
{
    string wikiname("Test Wiki Name");
    stringstream os;
    HTMLOutputterWikiString outputter(os, WikiPtr(this), wikiname);
    outputter.AddWikiLink(target_file, target_file);
    cout << os.str() << endl;
}


void Wiki::MarkContentDirty( string target_file )
{
    WikiEntryPtr entry;
    if (wikidb->LoadWikiEntry(entry,target_file))
    {
        entry->needsContentRebuild = true;
        wikidb->WriteWikiEntry(entry);
    }
    else
    {
        cerr << "Unable to find entry for '" << target_file << "'" << endl;
    }
}
void Wiki::MarkReferencesDirty( string target_file )
{
    WikiEntryPtr entry;
    if (wikidb->LoadWikiEntry(entry, target_file))
    {
        entry->needsExternalRebuild = true;
        wikidb->WriteWikiEntry(entry);
    }
    else
    {
        cerr << "Unable to find entry for '" << target_file << "'" << endl;
    }
}

void Wiki::ShowWikiStatus( string target_file )
{
    WikiEntryPtr entry;
    if (wikidb->LoadWikiEntry(entry, target_file))
    {
        cout << "wikiname: " << entry->wikiname << endl;
        cout << "inputname: " << entry->inputname << endl;
        cout << "needsContentRebuild: " << entry->needsContentRebuild << endl;
        cout << "needsExternalRebuild: " << entry->needsExternalRebuild << endl;

        string thisWikiName(entry->wikiname);

        cout << "From links:" << endl;
        {
            vector<WikiEntryReferencePtr> wikiEntryReferences;
            wikidb->LoadWikiReferencesFrom(wikiEntryReferences, thisWikiName);

            vector<string> currentLinks;
            for (auto wikiEntryReference = wikiEntryReferences.begin();
                 wikiEntryReference != wikiEntryReferences.end();
                 ++wikiEntryReference)
            {
                cout << "   " << (*wikiEntryReference)->to_wikiname << endl;
            }
        }
        cout << "To links:" << endl;
        {
            vector<WikiEntryReferencePtr> wikiEntryReferences;
            wikidb->LoadWikiReferencesTo(wikiEntryReferences, thisWikiName);

            vector<string> currentLinks;
            for (auto wikiEntryReference = wikiEntryReferences.begin();
                 wikiEntryReference != wikiEntryReferences.end();
                 ++wikiEntryReference)
            {
                cout << "   " << (*wikiEntryReference)->to_wikiname << endl;
            }
        }

        
    }
    else
    {
        cout << "Not Found: '" << target_file << "'" << endl;
    }

}


void Wiki::LoadEXIFData(string target_file )
{
    LoadJPEGData(target_file);
}
void Wiki::ScanDPLFiles()
{
    wikidb->BeginTransaction();
    vector<WikiEntryPtr> wikientries;
    string sql("SELECT * FROM WikiEntry");
    wikidb->LoadAllWikiEntries(wikientries);
    for(auto wikientry = wikientries.begin();
        wikientry != wikientries.end();
        ++wikientry)
    {
        string imagename;
        if (!StartsWithImage((*wikientry)->wikiname, imagename))
        {
            string s(LoadWikiText(*wikientry));

            if (s.find("<dpl") != string::npos
                || s.find("<DPL") != string::npos)
            {
                cout << "Found dpl in " << (*wikientry)->wikiname <<endl;
                (*wikientry)->needsContentRebuild = true;
                wikidb->WriteWikiEntry(*wikientry);
            }
        }
    }

    RebuildDirtyFiles(staging_area);
    CopyChangedFiles(staging_area, output_area);
    wikidb->EndTransaction();
}

void Wiki::ScanDPLFiles_NOCHANGES()
{
    wikidb->BeginTransaction();
    vector<WikiEntryPtr> wikientries;
    string sql("SELECT * FROM WikiEntry");
    wikidb->LoadAllWikiEntries(wikientries);
    for(auto wikientry = wikientries.begin();
        wikientry != wikientries.end();
        ++wikientry)
    {
        string s(LoadWikiText(*wikientry));
        if (s.find("<dpl") != string::npos)
        {
            (*wikientry)->needsContentRebuild = true;
            wikidb->WriteWikiEntry(*wikientry);
        }
    }

    wikidb->EndTransaction();
}


void Wiki::RebuildOutputFiles()
{
    wikidb->BeginTransaction();
    vector<WikiEntryPtr> wikientries;

    wikidb->LoadContentDirtyWikiEntries(wikientries);

    for(auto wikientry = wikientries.begin();
        wikientry != wikientries.end();
        ++wikientry)
    {
        try {
            ScanWikiFileForLinks((*wikientry)->inputname);
        }
        catch (FbyBaseExceptionPtr e)
        {
            cerr << e->file << ":" << e->line << " : " << e->Message << endl;
        }
    }
    RebuildDirtyFiles(staging_area);
    wikidb->EndTransaction();
}


void Wiki::TestStuff()
{
    string s("Sunday morning's sunrise.");

    TreeBuilderWiki treeBuilder(google_maps_api_key, wikidb);
    {
        cout << "Attempting to parse: '" << s << "'" << endl;
        MarkedUpTextParser treeParser;
        treeParser.Parse(treeBuilder,
                         s.c_str(),
                         s.size());
    }
    stringstream os;
    HTMLOutputterWikiString outputter(os, WikiPtr(this),  string("Random Name"));
    treeBuilder.AsHTML(outputter);
    cout << os.str() << endl;;
}
