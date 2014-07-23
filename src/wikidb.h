#ifndef WIKIDB_H_INCLUDED
#define WIKIDB_H_INCLUDED
#include "fbydb.h"
#include "fbyregex.h"
#include "wikiobjects.h"

using namespace std;

class WikiDB : public ::FbyHelpers::BaseObj
{
private:
    WikiDB(const WikiDB &);
    WikiDB & operator= (const WikiDB &);
protected:
    FbyDBPtr db;
    bool debug_output;

public:
WikiDB(FbyDBPtr db) : BaseObj(BASEOBJINIT(WikiDB)), db(db), debug_output(false) {}

    void NukeDatabase();
    void LoadDirtyWikiEntries(vector<WikiEntryPtr> &wikientries);
    void LoadAllWikiEntries(vector<WikiEntryPtr> &wikientries);
    void BeginTransaction();
    void EndTransaction();
    bool LoadImage(ImagePtr &image, const string &imagename);
    bool LoadOrCreateImage(ImagePtr &image, const string &imagename);
    bool ImageHasInstances(ImagePtr image);
    bool LoadImageInstance(ImageInstancePtr &imageInstance, const string &imagepath);
    ImageInstancePtr ImageInstanceFullsize(ImagePtr image);
    ImageInstancePtr ImageInstanceThumb(ImagePtr image);
    ImageInstancePtr ImageInstanceLightboxZoom(ImagePtr image);
    ImageInstancePtr ImageInstanceOriginal(ImagePtr image);
    bool LoadOrCreateImageInstance(ImageInstancePtr &imageInstance, const string &imagepath);
    bool LoadWikiEntry(WikiEntryPtr &wikiEntry, const string &name);
    bool LoadOrCreateWikiEntry(WikiEntryPtr &wikiEntry, const string &name);
    vector<ImageInstancePtr> ImageInstances(ImagePtr image);
    void WriteWikiEntry(WikiEntryPtr &wikiEntry);
    void WriteImageInstance(ImageInstancePtr imageInstance);
    void LoadWikiReferencesFrom(vector<WikiEntryReferencePtr> &wikiEntryReferences, const string &wikiname);
    void LoadWikiReferencesTo(vector<WikiEntryReferencePtr> &wikiEntryReferences, const string &wikiname);
    void DeleteWikiReference(const string &fromWikiName, const string &toWikiName);
    void AddWikiReference(const string &fromWikiName, const string &toWikiName);
    void LoadStatusUpdates(vector<StatusUpdatePtr> &updates,
                           const string &xid);
    void LoadStatusUpdates(vector<StatusUpdatePtr> &updates,
                           const string &from,
                           const string &to);
    void LoadDPLEntries(vector<WikiEntryPtr> & entries,
                        const string &category,
                        const string &order,
                        const string &pattern,
                        const string &count);
};

#endif /* WIKIDB_H_INCLUDED */
