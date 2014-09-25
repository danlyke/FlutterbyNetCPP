#ifndef WIKI_H_INCLUDED
#define WIKI_H_INCLUDED

#include "fbydb.h"
#include "parsetree.h"
#include "wikiobjects.h"

FBYCLASSPTR(WikiDB);

FBYCLASSPTR(Wiki);
FBYCLASS(Wiki) : public ::FbyHelpers::BaseObj
{
private:
    Wiki(const Wiki &);
    Wiki & operator= (const Wiki &);
protected:
    WikiDBPtr wikidb;
    string LoadWikiText(WikiEntryPtr wikientry);
    void ParseWikiBufferToOutput(string wikiname, const char *buffer, size_t length, const char *outputdir);
    string WebPathFromFilename(const string &filename);
    string ImageNameFromImageFileName(const string & filename);

    string LoadFileToString(const char *filename);
public:
    Wiki(FbyDBPtr);
    void ScanWikiFiles(const char *inputDir, const char *stagingDir);
    void ScanWikiFiles(const std::string &inputDir, const std::string &stagingDir)
    { ScanWikiFiles(inputDir.c_str(), stagingDir.c_str()); }
    void LoadJPEGData(const std::string &imagepath);
    void LoadPNGData(const std::string &imagepath);
    void CreateWikiPage();
    void GetImageHTML(std::ostream &os, 
                      const std::string &thisWikiName, 
                      const std::string & wikiname,
                      const std::string & imagename,
                      const string &text);

    void ScanWikiEntryForLinks(WikiEntryPtr wikiEntry);

    void ScanWikiFileForLinks(const char *filename);
    void ScanWikiFilesForLinks(const char *inputDir, const char *stagingDir);
    void ParseAndOutputFile(const char *inputfile, const char *outputdir);

    void ScanWikiFileForLinks(const std::string &filename)
    { ScanWikiFileForLinks(filename.c_str()); }
    void ScanWikiFilesForLinks(const std::string &inputDir, const std::string &stagingDir)
    { ScanWikiFilesForLinks(inputDir.c_str(), stagingDir.c_str()); }
    void ParseAndOutputFile(const std::string &inputfile, const std::string &outputdir)
    { ParseAndOutputFile(inputfile.c_str(), outputdir.c_str()); }

    bool Exists(const string & wikiname);
    void LoadDPLEntries(vector<WikiEntryPtr> & entries,
                        const string &category,
                        const string &order,
                        const string &pattern,
                        const string &count);
    void RebuildDirtyFiles(const char *outputdir);
    void RebuildDirtyFiles(const std::string &outputdir)
    { RebuildDirtyFiles(outputdir.c_str()); }

protected:
    int ScanImages(const char *path);
    int ScanImages(const std::string &path)
    { return ScanImages(path.c_str()); }
public:
    void BeginTransaction();
    void EndTransaction();

    void DoEverything();
    void DoWikiFiles();
    void GetWikiFiles();
    void WriteWikiFile(string target_file);
    void WriteImageFile(string target_file);
    void ReadWikiFile(string target_file);
    void RebuildDetached();
    void ImportKML(string target_File);
    void DoDirtyFiles();
    void DoChangedFiles();
    void LoadEXIFData(string target_file);
    void ScanWikiFiles();
    void ScanImages();
    void ScanDPLFiles();
    void VerifyWikiLink(string target_file);

    void ShowWikiStatus( string target_file );
    void MarkContentDirty( string target_file );
    void MarkReferencesDirty( string target_file );
    void DoWikiFile(string wikiname);

    void SetInputDirectory(string input_directory);
    void SetStagingDirectory(string staging_directory);
    void SetOutputDirectory(string output_directory);
    void SetGoogleMapsAPIKey(string google_maps_api_key);

// New cleaned up functions
    void GetContentDirty();
    void GetReferencedDirty();
    void RebuildOutputFiles();

    void ScanWikiFiles_NOCHANGES();
    void ScanDPLFiles_NOCHANGES();


// Random
    void TestStuff();

private:
    string staging_area;
    string output_area;
    string input_area;
    string google_maps_api_key;
};

class HTMLOutputterString : public HTMLOutputter {
protected:
    std::ostream &os;
public:
    HTMLOutputterString(ostream &os);
    virtual void AddString(const string &s);
    virtual void AddHTMLNodeBegin(const string &name, const vector< pair<string,string> > & attributes, bool empty_node);
    virtual void AddHTMLNodeEnd(const string &name);
    virtual void AddWikiLink(const string &wikiname, const string &text);
    virtual ~HTMLOutputterString();
};

class HTMLOutputterWikiString : public HTMLOutputterString  {
private:
    WikiPtr wiki;
    std::string thisWikiName;
public:
HTMLOutputterWikiString(ostream &os, WikiPtr wiki, const string &thisWikiName)
    : HTMLOutputterString(os), wiki(wiki), thisWikiName(thisWikiName) {}
    virtual void AddWikiLink(const string &wikiname, const string &text);
    virtual void AddDPLList(const string &category,
                            const string &order,
                            const string &pattern,
                            const string &count);
};


class HTMLOutputterOutboundLinks : public HTMLOutputter {
private:
    std::vector<std::string> wikiLinks;
public:
    HTMLOutputterOutboundLinks();
    virtual void AddWikiLink(const string &wikiname, const string &text);
    const vector<string> &GetLinks() { return wikiLinks; }
};

#endif /* WIKI_H_INCLUDED */
