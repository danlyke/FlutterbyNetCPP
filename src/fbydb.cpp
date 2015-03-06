#include "fby.h"
#include "fbydb.h"
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <time.h>
#include <sstream>

#include <limits.h>
#include <float.h>

using namespace std;

time_t TextDateToTime(const string &textDate)
{
    struct tm t;
    const char *lastchar;
    memset(&t, '\0', sizeof(t));
    lastchar = strptime(textDate.c_str(), "%Y-%m-%d %H:%M:%S%z", &t);
    
    if (!(lastchar && !*lastchar))
    {
        memset(&t, '\0', sizeof(t));
        lastchar = strptime(textDate.c_str(), "%Y-%m-%d %H:%M:%S", &t);
    
        if (!(lastchar && *lastchar == '.'))
        {
            if (!(lastchar && !*lastchar))
            {
                memset(&t, '\0', sizeof(t));
                lastchar = strptime(textDate.c_str(), "%Y-%m-%d", &t);
                if (!(lastchar && !*lastchar))
                { 
                    memset( &t, '\0', sizeof(t) );
                    // THROWEXCEPTION("Unrecognized date format '" + textDate + "'");
                }
            }
        }
    }

    t.tm_isdst = 0; // Timezone should have been gotten in strptime
    time_t retTime(timegm(&t));
    return retTime;
}

string TimeToTextDate(time_t t)
{
    struct tm *ptm = gmtime(&t);
    char timebuf[32];

    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S%z", ptm);
    return string(timebuf);
}


bool wrapper_stobool(const string &in)
{
    if (in.empty() || in == "f" || in == "0")
        return false;
    return true;
}


int wrapper_stoi(const string &in)
{
    if (in.empty())
        return INT_MIN;
    return stoi(in);
}

long wrapper_stol(const string &in)
{
    if (in.empty())
        return INT_MIN;
    return stol(in);
}

long wrapper_stoll(const string &in)
{
    if (in.empty())
        return INT_MIN;
    return stoll(in);
}

float wrapper_stof(const string &in)
{
    if (in.empty())
        return FLT_MAX;
    return stof(in);
}

double wrapper_stod(const string &in)
{
    if (in.empty())
        return FLT_MAX;
    return stod(in);
}

long wrapper_stold(const string &in)
{
    if (in.empty())
        return LONG_MAX;
    return stold(in);
}




FbyORM::FbyORM(const char *name, int size) :
    ::FbyHelpers::BaseObj(name, size),
    loaded(false)
{
}

FbyORM::~FbyORM()
{
}



FBYCLASS(FbyORMAnonymous) : public FbyORM
{
public:
    FbyORMAnonymous(const char *name, int size);
    
    virtual ~FbyORMAnonymous();
    virtual const char **MemberNames();
    virtual const char *ClassName();
    virtual const char **KeyNames();
};

FBYCLASSPTR(FbyORMHash);
FBYCLASS(FbyORMHash) : public FbyORMAnonymous
{
public:
    std::map< std::string, std::string > values;
public:
    FbyORMHash();
    void AssignToMember(const std::string & memberName,
                        const std::string & value);
    std::string AssignFromMember(const std::string &memberName);
};

FBYCLASSPTR(FbyORMArray);
FBYCLASS(FbyORMArray) : public FbyORMAnonymous
{
public:
    std::vector< std::string > values;
public:
    FbyORMArray();
    void AssignToMember(const std::string & memberName,
                        const std::string & value);
    std::string AssignFromMember(const std::string &memberName);
};





FbyORMAnonymous::FbyORMAnonymous(const char *name, int size) : FbyORM(name, size)
{
}

FbyORMAnonymous::~FbyORMAnonymous()
{
}

const char **FbyORMAnonymous::MemberNames()
{
    static const char **names = {NULL};
    return names;
}
const char *FbyORMAnonymous::ClassName()
{
    return NULL;
}
const char **FbyORMAnonymous::KeyNames()
{
    static const char **names = {NULL};
    return names;
}




FbyORMHash::FbyORMHash() :
    FbyORMAnonymous(BASEOBJINIT(FbyORMHash)),
    values()
{  
}

void FbyORMHash::AssignToMember(const std::string & memberName,
                    const std::string & value)
{
    values[memberName] = value;
}

std::string FbyORMHash::AssignFromMember(const std::string &memberName)
{
    return values[memberName];
}

FbyORMArray::FbyORMArray()
    : FbyORMAnonymous(BASEOBJINIT(FbyORMArray)), values()
{
}

void FbyORMArray::AssignToMember(const std::string & /* memberName */,
                    const std::string & value)
{
    values.push_back(value);
}

std::string FbyORMArray::AssignFromMember(const std::string &memberName)
{
    string str("Attempt to get value from anonymous array in FbyORMArray: " + memberName);
    THROWEXCEPTION(str);
}




string FbyDB::Quote(const string &s)
{
    string r("'");
    size_t pos;
    size_t prevPos = 0;
    const char *apos = "'";

    pos = s.find(apos);
    while (pos != string::npos)
    {
        r = r + s.substr(prevPos, pos) + apos;
        prevPos = pos;
        pos = s.find(apos, pos + 1);
    }
    r += s.substr(prevPos) + "'";
    return r;
}


string FbyDB::GetUpdateSQL(FbyORMPtr obj)
{
    string sql("UPDATE ");
    sql += obj->ClassName();
    sql += " SET ";
    const char **names = obj->MemberNames();
    bool first = true;

    for (int i = 0; names[i]; ++i)
    {
        if (string(names[i]) != "id")
        {
            if (!first)
                sql += ", ";
            first = false;

            
            sql += names[i];
            sql += "=" + Quote(obj->AssignFromMember(names[i]));
      
        }
    }
    sql += " WHERE ";

    first = true;
    const char **keys = obj->KeyNames();
    for (int i = 0; keys[i]; ++i)
    {
        if (!first)
            sql += " AND ";
        first = false;

        sql += keys[i];
        sql += "=" + Quote(obj->AssignFromMember(keys[i]));
    }
	return sql;
}


string FbyDB::GetInsertSQL(FbyORMPtr obj)
{
	string sql("INSERT INTO ");
    sql += obj->ClassName();
    sql += " (";
    const char **names = obj->MemberNames();
    bool first = true;

    for (int i = 0; names[i]; ++i)
    {
        if (string(names[i]) != "id")
        {
            if (!first)
                sql += ", ";
            first = false;
            sql += names[i];
        }
    }


    first = true;
    sql += ") VALUES (";
    for (int i = 0; names[i]; ++i)
    {
        if (string(names[i]) != "id")
        {
            if (!first)
                sql += ", ";
            first = false;
            sql += Quote(obj->AssignFromMember(names[i]));
        }
    }
    sql += ")";
	return sql;
}


void FbyDB::Write(FbyORMPtr obj)
{
    string sql(obj->WasLoaded() ? GetUpdateSQL(obj) : GetInsertSQL(obj));
//    cout << "Write: " << sql << endl;
    Do(sql);
    obj->SetLoaded();
//
//	if (defined($obj->id))
//	{
//		my $sql = $self->get_update_sql($obj, $attrs);
//		print "$sql\n"
//			if ($self->debug);
//		$self->do($sql);
//	}
//	else
//	{
//		my $sql = $self->get_insert_sql($obj, $attrs);
//		print "$sql\n"
//			if ($self->debug);
//
//		$anyway ? $self->do_anyway($sql) : $self->do($sql);
//		if ($self->connectargs()->[0] =~ /^DBI:Pg:/)
//		{
//			my @a = $self->dbh()->selectrow_array("SELECT CURRVAL(pg_get_serial_sequence('"
//												  .$self->tablename($obj)
//												  ."', 'id'))");
//			$obj->id($a[0]);
//		}
//		else
//		{
//			my $lastrowid = $self->dbh()->last_insert_id(undef,undef,$self->tablename($obj),'id');
//
//			die "Failed to get row id!\n".CallStack() if (!defined($lastrowid));
//			$obj->id($lastrowid);
//		}
//	}
}

void FbyDB::Do(const std::string &s)
{
    Do(s.c_str());
}

void FbyDB::Insert(const char *table, std::vector<std::string> &keys, std::vector<std::string> &values)
{
    stringstream ss;

    if (keys.size() != values.size())
    {
        ss << "Insert into ";
        ss << table;
        ss << " key (";
        ss << keys.size();
        ss << ") values (";
        ss << values.size();
        ss << ")";
        THROWEXCEPTION(ss.str());
    }
    else
    {
        ss << "INSERT INTO ";
        ss << table;
        ss << "(";

        for (auto s = keys.begin();
             s != keys.end();
             ++s)
        {
            if (s != keys.begin()) ss << ",";
            ss << *s;
        }

        ss << ") VALUES (";
        for (auto s = values.begin();
             s != values.end();
             ++s)
        {
            if (s != values.begin()) ss << ",";
            ss << Quote(*s);
        }
        ss << ")";
        Do(ss.str());
    }
}


void FbyDB::Begin() { Do("BEGIN;"); }
void FbyDB::End() { Do("END;"); }


int FbyDB::selectrows_array(std::vector< std::vector< std::string > > &values,
                            const char *s)
{
    vector<FbyORMArrayPtr> array;
    Load(array, s);
    values.reserve(array.size());
    for (auto i = array.begin();
         i != array.end();
         ++i)
    {
        values.push_back((*i)->values);
    }
    return values.size();
}

int FbyDB::selectrows_hash(std::vector< std::map< std::string, std::string > > &values, const char *s)
{
    vector<FbyORMHashPtr> hash;
    Load(hash, s);
    for (auto i = hash.begin(); i != hash.end(); ++i)
    {
        values.push_back((*i)->values);
    }
    return values.size();
}

bool FbyDB::selectrow_array(std::vector< std::string > &values, const char *s)
{
    FbyORMArrayPtr arr;
    bool loaded(LoadOne(arr, s));

    if (loaded)
    {
        values.reserve(arr->values.size());
        for (auto i = arr->values.begin(); i != arr->values.end(); ++i)
        {
            values.push_back(*i);
        }
    }
    return loaded;
}

bool FbyDB::selectrow_hash(std::map< std::string, std::string > &values, const char *s)
{
    FbyORMHashPtr hash;
    bool loaded(LoadOne(hash, s));
    if (loaded)    
    {
        for (auto i = hash->values.begin(); i != hash->values.end(); ++i)
        {
            values[i->first] = i->second;
        }
    }
    return loaded;
}
