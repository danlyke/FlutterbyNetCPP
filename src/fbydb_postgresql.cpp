#include <string>
#include <iostream>

#include <sstream>
#include "fbydb.h"

using namespace std;

FbyPostgreSQLDB::FbyPostgreSQLDB(const char *connection_string) : 
    FbyDB(BASEOBJINIT(FbyPostgreSQLDB)),
    psql(nullptr)
{
    psql = PQconnectdb(connection_string);
/* init connection */
    if (!psql) {
        string errmsg("libpq error: PQconnectdb returned NULL.\n\n");
        cerr << errmsg;
        THROWEXCEPTION(errmsg);
    }
    if (PQstatus(psql) != CONNECTION_OK) {
        string errmsg("libpq error: PQstatus(psql) != CONNECTION_OK: ");
        errmsg += PQerrorMessage(psql);
        cerr << errmsg << endl;
        THROWEXCEPTION(errmsg);
    } 
}

FbyPostgreSQLDB::~FbyPostgreSQLDB()
{
    PQfinish(psql);
    psql = nullptr;
}

DYNARRAY(FbyORMPtr) FbyPostgreSQLDB::Load( std::function<FbyORMPtr (void)> generator,
                                       const char *whereclause)
{
    PGresult *result;
    result = PQexec(psql, whereclause);

    switch (PQresultStatus(result))
    {
    case PGRES_COMMAND_OK:
    case PGRES_TUPLES_OK:
    case PGRES_COPY_OUT:
    case PGRES_COPY_IN:
        break;
    default:
    {
        stringstream errmsg;
        errmsg << "::Load SQL ERROR: (" << PQresultStatus(result) << ") " << PQerrorMessage(psql) << endl << whereclause << endl;
        cerr << errmsg.str() << endl;
        PQclear(result);
        THROWEXCEPTION(errmsg.str());
    }
    }

    DYNARRAY(FbyORMPtr) array;
    if (result)
    {
        int ntuples = PQntuples(result);
        int nfields = PQnfields(result);

        array = FBYNEWDYNARRAY(FbyORMPtr, ntuples);
        for (int row = 0; row < ntuples; ++row)
        {
            FbyORMPtr obj((generator)());

            for (int column = 0; column < nfields; ++column)
            {
                obj->AssignToMember(PQfname(result, column),
                                   PQgetvalue(result, row, column));
            }
            obj->SetLoaded();
            array[row] = obj;
        }
        PQclear(result);
    }
    else
    {
        array = FBYNEWDYNARRAY(FbyORMPtr, 0);
    }
    return array;
}



void FbyPostgreSQLDB::Do( const char *s )
{
    PGresult *result;

    result = PQexec(psql, s);
    if (result)
    {
        if (PQresultStatus(result) != PGRES_COMMAND_OK)
        {
            stringstream errmsg;
            errmsg << "SQL ERROR: " << PQerrorMessage(psql) << endl << s << endl;;
            cerr << errmsg.str() << endl;
            PQclear(result);
            THROWEXCEPTION(errmsg.str());
        }
        PQclear(result);
    }
}
