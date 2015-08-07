

#include <stdlib.h>
#include <stdio.h>
#include "sqlite3.h"


/* All uses of sqlite_exec() REQUIRE a callback function.
 * This function outputs the row data to stdout with tab delimiters.
 * <pArg> is an app-supplied argument passed into sqlite_exec().
 * <argc> is the number of columns of data.
 * <argv> is an array of the column data.
 * <azColName> is an array of the column names.
 */

static int callback_null(void *pArg, int argc, char **argv, char **azColName){
    return 0;
}

static int callback_desc_table(void *pArg, int argc, char **argv, char **azColName){
	int i;
	for (i = 0; i < argc; i++){
        printf("%s    %s\n", azColName[i], argv[i]);
	}
	return 0; /* okay */
}



sqlite3 * database_open(){
    //sqlite3_open("data.db", &db);
    
    sqlite3 *db = NULL;
    int err = sqlite3_open_v2("data.db", &db, SQLITE_OPEN_READWRITE, NULL);
    
    if (err){
        err = sqlite3_open_v2("data.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        if (err) {
            printf("could not open or create database\n");
            return NULL;
        }
        printf("initializing new database\n");
        sqlite3_exec(db, "create table media(id integer primary key, location text );", callback_null, NULL, NULL);
    }
    
    return db;
}

void database_desc_table(sqlite3 * db){
    sqlite3_exec(db, "SELECT type AS 'Type', name AS 'Name' FROM SQLITE_MASTER WHERE type='table' OR type='view';", callback_desc_table, NULL, NULL);
}




int main (int argc, char ** args){
    sqlite3 *db = database_open();
    
    database_desc_table(db);
    
    sqlite3_close(db);
        
    return 0;
}









//~ int main(int argc, char **argv)
//~ {
	//~ sqlite *db;
	//~ char *zErrMsg = NULL;
	//~ int rc;
	//~ int mode = 0;
	//~ const char* zSql = "SELECT type AS 'Type', name AS 'Name' FROM SQLITE_MASTER WHERE type='table' OR type='view'; ";

	//~ /* Validate command-line arguments.
	 //~ * argv[0] will be the application name.
	 //~ */
	//~ if (argc != 2 || !argv[1])
	//~ {
		//~ fprintf(stderr, "usage: %s database\n", argv[0]);
		//~ return -1; /* error */
	//~ }

	//~ /* Open the database connection. The mode flag is unused (as of 2.7.3) */
	//~ db = sqlite_open(argv[1], mode, &zErrMsg);
	//~ if (!db)
	//~ {
		//~ fprintf(stderr, "%s: failed to open database '%s'.\n%s", argv[0], argv[1], zErrMsg);
		//~ sqliteFree(zErrMsg);
		//~ return -1; /* error */
	//~ }

	//~ /* Execute the query. The work is done in the callback function. */
	//~ rc = sqlite_exec(db, zSql, callback_output_tabbed, NULL, &zErrMsg);
	//~ if (rc != SQLITE_OK)
	//~ {
		//~ fprintf(stderr, "%s: failed to execute query.\n%s", argv[0], zErrMsg);

		//~ /* The error message must be freed. This only needs to be done if rc != SQLITE_OK. */
		//~ sqliteFree(zErrMsg);
	//~ }

	//~ sqlite_close(db);

	//~ return (rc == SQLITE_OK)? 0: -1;
//~ }





