/*
The blue programming language ("blue")
Copyright (C) 2007-2008  Erik R Lechak

email: erik@lechak.info
web: www.lechak.info

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


/*
This file provides the interface to SQLite and open source/public domain database.
SQLite is not licensed under the GPL.
For more information on SQLite see:   http://www.sqlite.org
*/

#include "../../global.h"
#include "sqlite/sqlite3.h"

EXPORT void init(INITFUNC_ARGS);
static NativeType sqlite_type;
static NativeType record_type;


struct Sqlite{
    sqlite3 *db;    
};
typedef struct Sqlite * Sqlite;


static void  sqlite_create(Link self){
    Sqlite sql = self->value.vptr;
    sql->db = NULL;
}

static void sqlite_destroy(Link self){
    Sqlite sql = self->value.vptr;
    if ( sql->db ) sqlite3_close(sql->db); 
    object_destroy(self);
}

static NATIVECALL(sql_open){
    Link * args = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);
    
    Link sql_link = object_create(sqlite_type);
    Sqlite sql = sql_link->value.vptr;
    int err = sqlite3_open(object_getString(args[0])->data, &(sql->db));
    
    if (err) return exception("SQLOpenError",NULL,NULL);
    
    return sql_link;
};


static int callback_convert(void *pArg, int argc, char **argv, char **azColName){
    Link array = (Link) pArg;
    Link record = object_create(record_type);
    int i;
    for (i = 0; i < argc; i++){
        //printf("%s    %s\n", azColName[i], argv[i]);
        addAttr(record, create_string(argv[i]) , create_string(azColName[i]));
    }
    array_push(array, record);
    return 0; /* okay */
}

static NATIVECALL(sql_exec){
    Link * args = array_getArray(Arg);
    //size_t argn  = array_getLength(Arg);
    Link array = array_new(0);
    Sqlite sql = This->value.vptr;
    int err = sqlite3_exec(sql->db, object_getString(args[0])->data, callback_convert, (void *) array, NULL);
    if (err) return exception("SQLExecError",NULL,NULL);
    return array;
};


void init(INITFUNC_ARGS){

    sqlite_type = newNative();
        sqlite_type->create         = sqlite_create;
        sqlite_type->destroy        = sqlite_destroy;

    addCFunc(Module,"open", sql_open);
    addNativeCall(sqlite_type, "exec", sql_exec);
    
    record_type = newNative();
    
    //~ addNativeCall(dict_type, "keys", dict_keys);
    //~ addNativeCall(dict_type, "values", dict_values);
}





