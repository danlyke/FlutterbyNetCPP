#include "wikidb.h"

void WikiDB::NukeDatabase()
{
    static const char *tables[] = {
//        "Camera",
        "ImageInstance",
        "WikiEntryReference",
        "Image",
        "WikiEntry",
        "CameraTime",
        nullptr
    };
    for (const char **table = tables; *table; ++table)
    {
        string sql("DELETE FROM " + string(*table));
        db->Do(sql);
    }
}


void WikiDB::LoadDirtyWikiEntries(vector<WikiEntryPtr> &wikientries)
{
    string sql("SELECT * FROM WikiEntry WHERE (needsContentRebuild != 0) OR (needsExternalRebuild != 0)");
    db->Load(wikientries, sql.c_str());
}

void WikiDB::LoadContentDirtyWikiEntries(vector<WikiEntryPtr> &wikientries)
{
    string sql("SELECT * FROM WikiEntry WHERE (needsContentRebuild != 0)");
    db->Load(wikientries, sql.c_str());
}

void WikiDB::LoadReferencedDirtyWikiEntries(vector<WikiEntryPtr> &wikientries)
{
    string sql("SELECT * FROM WikiEntry WHERE (needsExternalRebuild != 0)");
    db->Load(wikientries, sql.c_str());
}


void WikiDB::LoadAllWikiEntries(vector<WikiEntryPtr> &wikientries)
{
    string sql("SELECT * FROM WikiEntry");
    db->Load(wikientries, sql.c_str());
}


void WikiDB::BeginTransaction()
{
    db->Do("BEGIN;");
}


void WikiDB::EndTransaction()
{
    db->Do("COMMIT;");
}


bool WikiDB::LoadImage(ImagePtr &image, const string &imagename)
{
    return db->LoadOne(image, imagename.c_str(), true);
}


bool WikiDB::LoadOrCreateImage(ImagePtr &image, const string &imagename)
{
    bool created = db->LoadOrCreate(image, imagename);

    if (created)
    {
        db->Write(image);
    }
    return created;
}


bool WikiDB::ImageHasInstances(ImagePtr image)
{
    return image->HasInstances(db);
}


bool WikiDB::LoadImageInstance(ImageInstancePtr &imageInstance, const string &imagepath)
{
    string sql("SELECT * FROM ImageInstance WHERE filename=" + db->Quote(imagepath));
    return db->LoadOne(imageInstance, sql);
}


ImageInstancePtr WikiDB::ImageInstanceFullsize(ImagePtr image)
{
    return image->Fullsize(db);
}


ImageInstancePtr WikiDB::ImageInstanceThumb(ImagePtr image)
{
    return image->Thumb(db);
}


ImageInstancePtr WikiDB::ImageInstanceLightboxZoom(ImagePtr image)
{
    return image->LightboxZoom(db);
}


ImageInstancePtr WikiDB::ImageInstanceOriginal(ImagePtr image)
{
    return image->Original(db);
}


bool WikiDB::LoadOrCreateImageInstance(ImageInstancePtr &imageInstance, const string &imagepath)
{
    return db->LoadOrCreate(imageInstance, imagepath);
    string sql("SELECT * FROM ImageInstance WHERE filename=" + db->Quote(imagepath));
    return db->LoadOne(imageInstance, sql);
}


bool WikiDB::LoadWikiEntry(WikiEntryPtr &wikiEntry, const string &name)
{
    return db->LoadOne(wikiEntry, name, true);
}


bool WikiDB::LoadOrCreateWikiEntry(WikiEntryPtr &wikiEntry, const string &name)
{
    if (db->LoadOrCreate(wikiEntry, name))
    {
        wikiEntry->needsContentRebuild = true;
        return true;
    }
    return false;
}


vector<ImageInstancePtr> WikiDB::ImageInstances(ImagePtr image)
{
    return image->Instances(db);
}


void WikiDB::WriteWikiEntry(WikiEntryPtr &wikiEntry)
{
    db->Write(wikiEntry);
}


void WikiDB::WriteImageInstance(ImageInstancePtr imageInstance)
{
    db->Write(imageInstance);
}


void WikiDB::LoadWikiReferencesFrom(vector<WikiEntryReferencePtr> &wikiEntryReferences, const string &wikiname)
{
    string sql("SELECT * FROM WikiEntryReference WHERE from_wikiname="
               + db->Quote(wikiname));
    db->Load(wikiEntryReferences, sql.c_str());
}


void WikiDB::LoadWikiReferencesTo(vector<WikiEntryReferencePtr> &wikiEntryReferences, const string &wikiname)
{
    string sql("SELECT * FROM WikiEntryReference WHERE to_wikiname="
               + db->Quote(wikiname));
    db->Load(wikiEntryReferences, sql.c_str());
}


void WikiDB::DeleteWikiReference(const string &fromWikiName, const string &toWikiName)
{
    string sql("DELETE FROM WikiEntryReference WHERE from_wikiname=");
    sql += db->Quote(fromWikiName) + " AND to_wikiname="
        + db->Quote(toWikiName);
    db->Do(sql);
    if (debug_output)
        cout << sql << endl;

    WikiEntryPtr wikiTo;
    if (db->LoadOne(wikiTo, toWikiName.c_str(), true))
    {
        if (!wikiTo->needsExternalRebuild)
        {
            wikiTo->needsExternalRebuild = true;
            db->Write(wikiTo);
        }
    }
}


void WikiDB::AddWikiReference(const string &fromWikiName, const string &toWikiName)
{
    vector<string> referenceKeys;
    referenceKeys.push_back(fromWikiName);
    referenceKeys.push_back(toWikiName);


    WikiEntryPtr wikiTo;
    if (LoadOrCreateWikiEntry(wikiTo, toWikiName)
        || !wikiTo->needsExternalRebuild)
    {
        if (debug_output)
            cout << "Marking '" << toWikiName << "' dirty" << endl;

        wikiTo->needsExternalRebuild = true;
        db->Write(wikiTo);
    }

    WikiEntryReferencePtr reference;

    if (db->LoadOrCreate(reference, referenceKeys))
    {
        if (debug_output)
            cout << "Writing reference from " << referenceKeys[0] << " to " << referenceKeys[1] << endl;

        db->Write(reference);
    }
}


void WikiDB::LoadStatusUpdates(vector<StatusUpdatePtr> &updates,
                       const string &xid)
{
    string sql("SELECT * FROM StatusUpdate WHERE xid=" + db->Quote(xid));
    db->Load(updates, sql.c_str());
}

void WikiDB::LoadStatusUpdates(vector<StatusUpdatePtr> &updates,
                       const string &from,
                       const string &to)
{
    string sql("SELECT StatusUpdate.*, Person.name AS person FROM StatusUpdate JOIN Person ON StatusUpdate.person_id = Person.id WHERE entered >="
               + db->Quote(from)
               + " AND entered <= "
               + db->Quote(to)
               + " ORDER BY id LIMIT 100");
    db->Load(updates, sql.c_str());
}

void WikiDB::LoadDPLEntries(vector<WikiEntryPtr> & entries,
                    const string &category,
                    const string &order,
                    const string &pattern,

                    const string &count)
{
    string sqlorder;
    string sql("SELECT wikientry.* FROM wikientry");

    if (!category.empty())
        sql +=
            " JOIN wikientryreference ON wikientryreference.from_wikiname=wikientry.wikiname"
            " WHERE wikientryreference.to_wikiname = "
            + db->Quote("Category:" + category);

//    if (!pattern.empty())
//        sql += " wikientry.wikiname LIKE " + db->Quote(pattern);
//    else
//        sql += " 1";

    sql += " ORDER BY wikientry.wikiname";
    if (order == "descending")
        sql += " DESC";

    if (!count.empty() && pattern.empty())
    {
        sql += " LIMIT " + count;
    }
    if (debug_output)
        cout << "Loading DPL entries " << sql << endl;


    if (pattern.empty())
    {
        db->Load(entries, sql.c_str());
    }
    else
    {
        Regex regex(pattern.c_str());
        RegexMatch match;
        

        vector<WikiEntryPtr> unfilteredEntries;
        db->Load(unfilteredEntries, sql.c_str());
            
        int toCopy = count.empty() ? unfilteredEntries.size() : stoi(count);
            

        for (auto entry = unfilteredEntries.begin();
             entry != unfilteredEntries.end() && toCopy > 0;
             entry++)
        {
            if (regex.Match((*entry)->wikiname, match))
            {
                entries.push_back(*entry);
                toCopy--;
            }
        }
    }
}

