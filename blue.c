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

#include "version.h"

#include "global.h"
#include "files.h"
#include "number.h"


#ifdef __WIN32__
#include <windows.h>
#endif


struct Global * Global = NULL; // Main global container

static string_t find_blue(){
    char * location      = NULL;
    string_t ret         = NULL;

#ifdef __WIN32__
    char * APPNAME = "blue.exe";
    HKEY entry;
    long length;
    long err;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software\\Blue\\Location" , 0, KEY_ALL_ACCESS, &entry );
    if( err != ERROR_SUCCESS ){
        printf("Could not locate Blue location \n");
        exit(1);
    }

    err = RegQueryValueEx( entry, NULL, NULL, NULL, NULL, &length );
    if( err != ERROR_SUCCESS ){
        printf("Could not get value \n");
        exit(1);
    }

    location = malloc(length +1);

    err = RegQueryValueEx( entry, NULL, NULL, NULL, location, &length );
    if( err != ERROR_SUCCESS ){
        printf("Could not get value\n");
        exit(1);
    }

    RegCloseKey(entry);
#else
    char * APPNAME = "blue";
    char * temp = NULL;
    int chars_read = 0;

    /* look for the location of the blue install in the environment variable BLUE */
        location = file_pathto(".", "BLUE");
        if (location) goto check;

    /* BLUE environment variable not set so we look in the path for the blue link */
        /* look in path for blue executable (it should be a link)*/
        temp = file_pathto("blue", "PATH");
        if (temp){
            location = malloc(256);
                if (! location){
                    printf("Out of Memory");
                    exit(1);
                }
                location[0] = 0;


            /* find where the link points to (it should point to blue in it's installation directory) */
            chars_read = readlink(temp,location, 255);
            if ( chars_read == -1){
                printf("Installation problem, %s is not readable or is not a link\n", temp);
                free(location);
                free(temp);
                exit(1);

            }else if (chars_read == 255){
                printf("Location buffer not large enough\n");
                free(location);
                free(temp);
                exit(1);
            }else{

                /* null terminate location */
                location[chars_read] = 0;

                /* free path to the blue link */
                free(temp);

                /* set location to point to blue installation dir not the blue executable in that dir */

                temp = file_cd_up(location);
                free(location);
                location = temp;
                goto check;
            }
        }

    /* Still can't find it, so look in the current directory */
        location = malloc(2);
        if (! location){
            printf("Out of Memory");
            exit(1);
        }
        strcpy(location, ".");

    check:
#endif

    /* Check location */
    if (! file_exists2(location, APPNAME)){
        printf("Can't find blue at %s\n", location);
        exit(1);
    }

    ret = string_new(location);
    free(location);
    return ret;
}

static string_t find_blue_lib(){
    char * blue_lib_location = file_cd_into(Global->blue_location->data, "lib");
    string_t ret = string_new(blue_lib_location);
    free(blue_lib_location);
    return ret;
}

static void global_init(){
    
    /* locate the installation directory */
    Global->blue_location       = find_blue();
    Global->blue_lib_location   = find_blue_lib();
    
    /* 0 = only main thread; >0 = multi-threaded */
    Global->threads = 0;

    /* initialize basic object functionality */
    object_init();
    native_init();

    Global->backtrace_key       = create_string("__backtrace");
    Global->constructor_key     = create_string("_");
    Global->destructor_key      = create_string("__");

    Link loc     = create_string(Global->blue_location->data);
    Link loc_key = create_string("install_dir");
    
    addAttr(Global->SYS_MODULE, loc , loc_key);
    link_free(loc);
    link_free(loc_key);
}

static void usage(){
    printf(BLUE_VERSION);
    printf(", Copyright 2007-2008 Erik Lechak\n");
    printf("distributed under the terms of the GNU General Public License.\n");
    printf("--------------------------\n");
    printf("USAGE:\n");
    printf("    blue [options] <filename> [arguments]\n");    
    printf(" options:\n");
    printf("    -a: assemble <filename> to bytecode (executable) \n");
    printf("    -c: compile <filename> to bytecode (executable)\n");
    printf("    -d: decompile <filename>\n");
    printf("    -g: debug information\n");
    printf("    -o: output used with -c and -a options\n");
    printf("    -s: compile  <filename> to assembly code (not executable)\n");
    printf(" arguments: these arguments will be passed to the program\n");
    exit(0);
}


static void showBacktrace(Link ret){
    if (! is_critical(ret)) return;
    
    string_t retstring = object_asString(ret);
    printf("%s\n", retstring->data);
    free(retstring);
    
    Link bt = getAttr(ret, Global->backtrace_key);
    
    if ( ! bt ) return;
    
    Link * args = array_getArray(bt);
    size_t argn  = array_getLength(bt);
    
    Link a;
    int c;
    
    for (c=0; c< argn; c++){
        a = args[c];
        if (! a ) break;
        retstring = object_asString(a);
        printf(" %s\n", retstring->data);
        free(retstring);
    }
    link_free(bt);
}



int blue_main(int argc, char **argv){
    
    int arg_index;
    int mode=0;
    char  * filename = NULL;
    char * output_filename = NULL;

    /* Not enough arguments */
    if (argc < 2 ) usage();

    Global = malloc(sizeof(struct Global));
        if ( ! Global){
            printf("Out of Memory\n");
            exit(1);
        }
    
    Global->dbg_status = 0;        

    /* handle arguments to blue */
    for (arg_index=1; arg_index<argc ; arg_index++){
        if (argv[arg_index][0] == '-'){
            
            /* disassemble and quit*/
            if ( strcmp(argv[arg_index], "-d")==0 ) mode = 1;
            else if ( strcmp(argv[arg_index], "-c")==0 ) mode = 2;
            else if ( strcmp(argv[arg_index], "-g")==0 ) Global->dbg_status = 1;
                
            else if ( strcmp(argv[arg_index], "-o")==0 ){
                arg_index++;
                output_filename = argv[arg_index];
            }
                
            else if ( strcmp(argv[arg_index], "-s")==0 ){
                mode = 3 ;
            }
            
            else if ( strcmp(argv[arg_index], "-a")==0 ){
                mode = 4 ;
            }            
                      
            
            if (argc <3) return 0;

        }else{
            filename = argv[arg_index];
            break;
        }
    }

    /* Initialization routines */
        global_init();
    
    link_setThreadState(0); // turn multi-threading off until needed

    if (mode==1){
    /* DISASSEMBLE */
        Link module = create_module_filename(filename);
        string_t code = disassemble(module->value.module->bytecode);
        string_fprint(code , stdout);
        exit(0);
    } else if (mode == 2){
    /* COMPILE */
        Link module = create_module_filename(filename);
        module_save( module,  output_filename ? output_filename : "blue.blx");
        exit(0);
    }else if (mode == 3){
    /* COMPILE TO ASSEMBLY CODE */
        char * acode = compile_file( filename);
        
        if ( memcmp( acode, "ERR:",4) ==0 ){
            printf( acode+4);
            exit(0);
        }
        
        if (output_filename){
            FILE * fp = fopen( output_filename,"wb");
            fprintf(fp,acode);
            fclose(fp);            
        }else{
            printf(acode);
        }
        free(acode);
        exit(0);        

    }else if (mode == 4){
    /* ASSEMBLE THS ASSEMBLY CODE TO BYTECODE */
        char * acode = file_load(filename);
        
        if ( memcmp( acode, "ERR:",4) ==0 ){
            printf( acode+4);
            exit(0);
        }        
        
        Bytes bytes = assemble( acode );
        if (output_filename){
            bytes_save(bytes, output_filename);
        }else{
            bytes_save(bytes, "out.blx");
        }
        free(acode);
        exit(0);
    }
    
    /* Add arguments from call to this application */
        Link Arg = array_new(argc-arg_index);
        Link * Argv = array_getArray(Arg);
        Link a = NULL;

        int c = 0;

        for(c=0; c <argc-arg_index; c++){
            a = create_string_str( string_new((argv+arg_index)[c]) );
            Argv[c] =a;
        }

    /* creates and runs the module */
    Link main_ret = module_new(filename, Arg);
    link_free(Arg);

    if (main_ret){                
        showBacktrace(main_ret);
        link_free(main_ret);
    }
    
    fflush(stdout);
    fflush(stderr);

    exit(0);
    return 0;
}

