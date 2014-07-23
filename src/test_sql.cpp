#include <iostream>
#include <string>
#include <sqlite3.h>

usign namespace std;

const char *usage = " [database] [query]";


static int callback(void *thisVoid, int argc, char **argv, char **azColName)
{
    for (int i = 0; i < argc; ++i)
    {
        cout << azColName[i] << " (" << to_string(i) ") : " << argv[i] << endl;
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        cerr << argv[0];
        cerr << usage;
        cerr << usage;
        return (-1);
    }

    sqlite3 *db = nullptr;
    int result_code = sqlite3_open(argv[1], &db);
    if (rc)
    {
        cerr << "Can't open database '" << argv[1] << "': "  << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return -1;
    }
    char *error_message;
    result_code = sqlite3_exec(db, argv[2], callback, nullptr, &error_message);
    if (result_code != SQLITE_OK)
    {
        cerr << "SQL error:" << endl << argv[2] << endl << sqlite3_errmsg(db) << endl;
    }

    sqlite3_close(db);
    return result_code != SQLITE_OK;
}

