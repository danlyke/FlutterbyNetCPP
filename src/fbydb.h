#ifndef FBYDB_H_INCLUDED
#define FBYDB_H_INCLUDED

#include "fby.h"
#include <time.h>
#include <string>
#include <functional>
#include <map>

extern time_t TextDateToTime(const std::string &textDate);
extern std::string TimeToTextDate(time_t t);
bool wrapper_stobool(const std::string &in);
int wrapper_stoi(const std::string &in);
long wrapper_stol(const std::string &in);
long wrapper_stoll(const std::string &in);
float wrapper_stof(const std::string &in);
double wrapper_stod(const std::string &in);
long wrapper_stold(const std::string &in);


#define FBYORM_SQLOBJECT private:\
    virtual void AssignToMember(const std::string & memberName,  \
                                const std::string & value);      \
    virtual std::string AssignFromMember(const std::string &memberName); \
    static const char **StaticMemberNames();                          \
    static const char **StaticKeyNames();                      \
    static const char * StaticClassName();                             \
    virtual const char **MemberNames();                          \
    virtual const char **KeyNames();                      \
    virtual const char *ClassName();                             \
    friend class FbyDB

FBYCLASSPTR(FbyDB);
FBYCLASSPTR(FbyORM);
FBYCLASS(FbyORM) : public FbyHelpers::BaseObj
{
private:
    bool loaded;
    FBYUNCOPYABLE(FbyORM);
public :
    FbyORM(const char *name, int size);
    virtual ~FbyORM();
    virtual void AssignToMember(const std::string & memberName,
                                const std::string & value) = 0;
    virtual std::string AssignFromMember(const std::string &memberName) = 0;
    virtual const char **MemberNames() = 0;
    virtual const char *ClassName() = 0;
    virtual const char **KeyNames() = 0;
    void SetLoaded() { loaded = true; }
    bool WasLoaded() const { return loaded; }
};


FBYCLASS(FbyStatement) : public FbyHelpers::BaseObj
{
public:
    FbyStatement(const char *name, int size);

};

using namespace FbyHelpers;

FBYCLASS(FbyDB) : public ::FbyHelpers::BaseObj
{
public:
    virtual ~FbyDB() {};
FbyDB(const char *name, int size) : BaseObj(name,size) {};
    void Write(FbyORM *obj);
    virtual DYNARRAY(FbyORMPtr) Load( std::function<FbyORMPtr (void)> generator,
                                   const char *whereclause) = 0;
    DYNARRAY(FbyORMPtr) Load( std::function<FbyORMPtr (void)> generator,
                              const std::string &whereclause)
    { return Load(generator, whereclause);}
    std::string Quote(const std::string &s);
    std::string GetUpdateSQL(FbyORMPtr obj);
    std::string GetInsertSQL(FbyORMPtr obj);
    void Write(FbyORMPtr obj);
    virtual void Do(const char *s) = 0;
    virtual void Do(const std::string &s);
    virtual void Begin();
    virtual void End();

    void Insert(const char *table, std::vector<std::string> &keys, std::vector<std::string> &values);

    int selectrows_array(std::vector< std::vector< std::string > > &values, const char *s);
    int selectrows_hash(std::vector< std::map< std::string, std::string > > &values, const char *s);

    bool selectrow_array(std::vector< std::string > &values, const char *s);
    bool selectrow_hash(std::map< std::string, std::string > &values, const char *s);


    int selectrows_array(std::vector< std::vector< std::string > > &values, const std::string &s)
    { return selectrows_array(values, s.c_str()); }
    int selectrows_hash(std::vector< std::map< std::string, std::string > > &values, const std::string &s)
    { return selectrows_hash(values, s.c_str()); }

    bool selectrow_array(std::vector< std::string > &values, const std::string &s)
    { return selectrow_array(values, s.c_str()); }
    bool selectrow_hash(std::map< std::string, std::string > &values, const std::string &s)
    { return selectrow_hash(values, s.c_str()); }

    std::string selectvalue(const char *s)
    {
        std::vector<std::string> a;
        selectrow_array(a, s);
        if (!a.empty())
            return a[0];
        return std::string();
    }
    std::string selectvalue(const std::string &s)
    {
        return selectvalue(s.c_str());
    }
    

    template <class C1, class C2> void CastCopyArray(DYNARRAY(C1) inData, DYNARRAY(FBYPTR(C2)) &outData)
    {
        for (int i = 0; i < inData->Count; ++i)
        {
            C1 obj = inData[i];
            FBYPTR(C2) d(dynamic_cast<C2 *>(&*obj));
            outData->Add(d);
        }
    }

    template <class C1, class C2> void CastCopyArray(DYNARRAY(C1) inData, std::vector<FBYPTR(C2)> &outData)
    {
        for (int i = 0; i < inData->Count; ++i)
        {
            C1 obj = inData[i];
            FBYPTR(C2) d(dynamic_cast<C2 *>(&*obj));
            outData.push_back(d);
        }
    }

    template <class C> void Load(DYNARRAY(FBYPTR(C)) &data, const char *whereclause)
    {
        DYNARRAY(FbyORMPtr) d =Load([](){ FbyORMPtr p(FBYNEW C()); return p; }, whereclause);
        CastCopyArray(d,data);
    }
    template <class C> void Load(std::vector<FBYPTR(C)> &data, const char *whereclause)
    {
        DYNARRAY(FbyORMPtr) d =Load([](){ FbyORMPtr p(FBYNEW C()); return p; }, whereclause);
        CastCopyArray(d,data);
    }

    template <class C> bool LoadOne(FBYPTR(C) &obj,
                                    const std::string &whereclause, bool addDefaultSQL)
    {
        return LoadOne(obj, whereclause.c_str(), addDefaultSQL);
    }

    template <class C> bool LoadOne(FBYPTR(C) &obj,
                                    const char *whereclause, bool addDefaultSQL)
    {
        const char **keynames = C::StaticKeyNames();
        assert(NULL != keynames[0]
               && NULL == keynames[1]);

        std::string sql(whereclause);
        if (addDefaultSQL)
        {
            sql = "SELECT * FROM ";
            sql += C::StaticClassName();
            sql += " WHERE ";
            sql += keynames[0];
            sql += "=" + Quote(whereclause);
        }
        return LoadOne(obj, sql.c_str());
        
    }

    template <class C> bool LoadOne(FBYPTR(C) &obj,
        const char *whereclause)
    {
        DYNARRAY(FBYPTR(C)) data;
        DYNARRAY(FbyORMPtr) loadedData(Load([](){ FbyORMPtr p(FBYNEW C()); return p; }, whereclause));
        FBYASSERT(loadedData->Length <= 1);
        CastCopyArray(loadedData,data);
        if (data->Length > 0)
            obj = data[0];
        else
            obj = FBYTYPEDNULL(FBYPTR(C));
        return data->Length != 0;
    }

    template <class C> bool LoadOne(FBYPTR(C) &obj,
                                    std::string sql)
    {
        return LoadOne(obj, sql.c_str());
    }

    template <class C> bool LoadOrCreate(FBYPTR(C) &obj,
        const std::string &primaryKey)
    {
        bool created = false;
        const char **keynames = C::StaticKeyNames();
        assert(NULL != keynames[0]
               && NULL == keynames[1]);
        std::string sql("SELECT * FROM ");
        sql += C::StaticClassName();
        sql += " WHERE ";
        sql += keynames[0];
        sql += " = " + Quote(primaryKey);
        
        if (!LoadOne(obj, sql))
        {
            FBYPTR(C) newObj(FBYNEW C);
            obj = newObj;
            obj->AssignToMember(std::string(keynames[0]),
                                primaryKey);
//            obj->ClearLoaded();
            created = true;
        }
        else
        {
            obj->SetLoaded();
        }
        return created;
    }

    template <class C> bool LoadOrCreate(FBYPTR(C) &obj,
                                         const std::vector<std::string> &primaryKeys)
    {
        bool created = false;
        size_t i;
        for (i = 0; i < primaryKeys.size(); ++i)
        {
            assert(C::StaticKeyNames()[i] != NULL);
        }
        assert(NULL == C::StaticKeyNames()[i]);

        std::string sql("SELECT * FROM ");
        sql += C::StaticClassName();
        sql += " WHERE ";

        bool first = true;
        for (i = 0; i < primaryKeys.size(); ++i)
        {
            if (!first)
            {
                sql += " AND ";
            }
            first = false;
            sql += C::StaticKeyNames()[i];
            sql += " = " + Quote(primaryKeys[i]);
        }
        
        if (!LoadOne(obj, sql.c_str()))
        {
            obj = FBYPTR(C) (FBYNEW C);
            for (i = 0; i < primaryKeys.size(); ++i)
            {
                obj->AssignToMember(std::string(C::StaticKeyNames()[i]),
                                    primaryKeys[i]);
            }
            created = true;
        }
        return created;
    }

};

FBYCLASSPTR(FbyDB);


#include <stdio.h>
extern "C" {
#include <sqlite3.h>
};


FBYCLASS(FbySQLiteDB) : public FbyDB
{
private:
    FbySQLiteDB(const FbySQLiteDB&);
    FbySQLiteDB &operator=(const FbySQLiteDB&);

public:
    std::function<FbyORMPtr (void)> currentGenerator;
    DYNARRAY(FbyORMPtr) currentDynarray;
    sqlite3 *db;
    
public:
    FbySQLiteDB(const char *);
    virtual ~FbySQLiteDB();
    DYNARRAY(FbyORMPtr) Load( std::function<FbyORMPtr (void)> generator,
                                   const char *whereclause);
    
    virtual void Do(const char *s);
};

#include <postgresql/libpq-fe.h>

FBYCLASS(FbyPostgreSQLDB) : public FbyDB
{
private:
    FbyPostgreSQLDB(const FbyPostgreSQLDB&);
    FbyPostgreSQLDB &operator=(const FbyPostgreSQLDB&);

public:
    PGconn *psql;
    
public:
    FbyPostgreSQLDB(const char *);
    virtual ~FbyPostgreSQLDB();
    DYNARRAY(FbyORMPtr) Load( std::function<FbyORMPtr (void)> generator,
                                   const char *whereclause);
    
    virtual void Do(const char *s);
};



#endif /* #ifndef FBYDB_H_INCLUDED */
