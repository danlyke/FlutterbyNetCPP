#include <sqlite3.h>
#include "fbydb.h"

#include <iostream>
#include <string>
using namespace std;

// FbySQLiteDB::LastInsertRowId()
// {
//     return sqlite3_last_insert_rowid(db);
// }


FbySQLiteDB::FbySQLiteDB(const char *dbname) : 
    FbyDB(BASEOBJINIT(FbySQLiteDB)),
    currentGenerator(),
    currentDynarray(),
    db(NULL)
{
    int rc;
     
    rc = sqlite3_open(dbname, &db);
    if( rc )
    {
        string errmsg = "Can't open database '" + string(dbname) + "': " + string(sqlite3_errmsg(db)) + "\n";
        fprintf(stderr, "%s", errmsg.c_str());
        sqlite3_close(db);
        THROWEXCEPTION(errmsg);
    }
}
FbySQLiteDB::~FbySQLiteDB()
{
    sqlite3_close(db);
}

static int callback(void *thisVoid, int argc, char **argv, char **azColName){
    int i;
    FbySQLiteDB *dbptr = static_cast<FbySQLiteDB *>(thisVoid);
    FbyORMPtr obj( (dbptr->currentGenerator)() );

    for(i=0; i<argc; i++)
    {
        if (argv[i])
        {
            obj->AssignToMember(string(azColName[i]), string(argv[i]));
        }
    }
    obj->SetLoaded();
    dbptr->currentDynarray->Add(obj);
    return 0;
}




DYNARRAY(FbyORMPtr) FbySQLiteDB::Load( std::function<FbyORMPtr (void)> generator,
                                       const char *whereclause)
{
    char *zErrMsg = 0;
    this->currentGenerator = generator;
    this->currentDynarray = FBYNEWDYNARRAY(FbyORMPtr, 0);
    int rc = sqlite3_exec(db, whereclause, callback, static_cast<void *>(this), &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n%s\n", zErrMsg,whereclause);
        sqlite3_free(zErrMsg);
    }
    return this->currentDynarray;
}



void FbySQLiteDB::Do( const char *s )
{
    char *zErrMsg = 0;
//    this->currentGenerator = generator;
    this->currentDynarray = FBYNEWDYNARRAY(FbyORMPtr, 0);
    int rc = sqlite3_exec(db, s, callback, static_cast<void *>(this), &zErrMsg);
    if( rc!=SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n%s\n", zErrMsg,s);
        sqlite3_free(zErrMsg);
    }
}
