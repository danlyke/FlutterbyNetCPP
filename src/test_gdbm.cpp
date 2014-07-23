#include <stdio.h>
#include <string.h>
#include <gdbm.h>
#include <stdlib.h>

/*
GDBM_FILE gdbm_open(name, block_size, flags, mode, fatal_func);
void gdbm_close(dbf);
int gdbm_store(dbf, key, content, flag);
datum gdbm_fetch(dbf, key);
int gdbm_delete(dbf, key);
datum gdbm_firstkey(dbf);
datum gdbm_nextkey(dbf, key);
int gdbm_reorganize(dbf);
void gdbm_sync(dbf);
int gdbm_exists(dbf, key);
char *gdbm_strerror(errno);
int gdbm_setopt(dbf, option, value, size);
int gdbm_fdesc(dbf);
*/

GDBM_FILE dbf;


int main (int /* argc */, char * /* argv */[])
{
    dbf = gdbm_open (const_cast<char*>("test.gdbm"), 0, GDBM_WRCREAT, 0666, 0);
   
    if (!dbf)
    {
        fprintf (stderr, "File either doesn't exist or is not a gdbm file.\n");
        return (2);
    }
    
    const char *datab[] =  {
        "key1", "value1",
        "key2", "value2",
        "key3", "value3",
        "key4", "value4",
        "key5", "value5",
        "key6", "value6",
        "key7", "value7",
        "key8", "value8",
        "key9", "value9",
        "key10", "value10",
        "key11", "value11",
        NULL
    };
    datum key, data;
    
    for (int i = 0; datab[i]; i += 2)
    {
        key.dsize = strlen(datab[i]) + 1;
        key.dptr = const_cast<char*>(datab[i]);
        data.dsize = strlen(datab[i + 1]) + 1;
        data.dptr = const_cast<char*>(datab[i + 1]);
        gdbm_store(dbf, key, data, GDBM_REPLACE);
    }

    for (int i = 0; datab[i]; i += 2)
    {
        key.dsize = strlen(datab[i]);
        key.dptr = const_cast<char*>(datab[i]);

        data = gdbm_fetch (dbf, key);
        if (data.dsize > 0) {
            printf ("%s\n", data.dptr);
            free (data.dptr);
        } else {
            printf ("Key %s not found.\n", key.dptr);
        }
    }
    gdbm_close (dbf);
    return 0;
}
