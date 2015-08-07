/*
The blue programming language ("blue")
Copyright (C) 2007  Erik R Lechak

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


#include "files.h"

#ifdef __WIN32__
#include <windows.h>
static const char SEP1 = ';';
static const char SEP2[] = "\\";

#else
static const char SEP1 = ':';
static const char SEP2[] =  "/";
#endif

#include <stdio.h>



char * file_load(const char * filename){
    FILE * fp =  fopen(filename , "rb");
    if (! fp) return NULL;
    fseek(fp, 0 , SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0 , SEEK_SET);
    
    char * self = malloc( file_size+1);
    
    fread(self , file_size, 1,fp);
    self[file_size] = 0;
    fclose(fp);    
    return self;
}


int file_copy(char * source, char * dest){
    char buffer[512];
    FILE * s = NULL;
    FILE * d = NULL;

    s = fopen(source,"r");
    if (! s ) return 1;

    d = fopen(dest,"w");
    if (! d){
        fclose(s);
        return 1;
    }

    while (fwrite (buffer, 1, fread (buffer, 1, 512, s), d) == 512);
    fclose(d);
    fclose(s);
    return 0;
}

char * file_name(char * filename){

    char * start = NULL;
    char * ret   = NULL;

    if ( ! *filename ) return NULL;

    //find the last slash
    start = strrchr(filename, *SEP2);

    if (start){ // found a slash
        //make sure it is a file and not a directory
        if ( ! *(++start) ) return NULL;

    } else{ // no slash so the filename is just the filename
        start = filename;
    }

    ret =  malloc(strlen(start)+1);
    if (! ret) return NULL;
    strcpy(ret,start);
    return ret;
}

int file_isexecutable(char * filename, char * env){
    char * path = NULL;

#ifdef __WIN32__

    char exe_suffix[4][5] ={ ".exe" , ".bat", ".com", ""};
    char * fullname = malloc(strlen(filename)+5);

    if (! fullname) return 0;

    int index = 0;
    while (exe_suffix[index][0]){
        strcpy(fullname, filename);
        strcat(fullname, exe_suffix[index]);
        path = file_pathto(fullname, env);
        if (path){
            free(path);
            free(fullname);
            return 1;
        }
        index++;
    }
    free(path);
    free(fullname);
    return 0;
#else
    int ret;

    path = file_pathto(filename, env);
    ret = access(path, X_OK) ? 0:1;
    free(path);
    return ret;
#endif
}


int file_exists2(char * part1, char * part2){
    size_t length = 0;

    if ( part1 ) length += strlen(part1);
    if ( part2 ) length += strlen(part2);

    if (! length) return 0;

    char * test = malloc( length + 20 );
    *test = 0;

    if (part1){
        strcat(test, part1);

        length = strlen(part1);

        if ( length && (test[length-1] != *SEP2) && part2 && (*part2 != *SEP2)){
            strcat(test, SEP2);
        }
    }

    if (part2) strcat(test, part2);

    int ret = file_exists(test);

    free(test);
    return ret;
}


char * file_cd_up(const char * path){
    //char * sep = NULL;
    char * ret = NULL;
    size_t length = 0;
    char slash = SEP2[0];

    if (! path) return NULL;

    length = strlen(path) - 1; // length is actually index into path

    /* if the path ends with a slash get rid of it */
    if (path[length] == slash) length--;

    /* look for rightmost slash */
    while(length){
        if (path[length] == slash) break;
        length --;
    }

    /* bail out if there was no slash */
    if (! length) return NULL;

    ret = malloc( length +1);
        if ( ! ret) {
            printf("Out of Memory\n");
            exit(1);
        }

    memcpy(ret, path, length);
    ret[length] = 0;
    return ret;
}





char * file_cd_into(char * part1, char * part2){
    size_t length = 0;

    if ( part1 ) length += strlen(part1);
    if ( part2 ) length += strlen(part2);

    if (! length) return 0;

    char * test = malloc( length + 20 );
    *test = 0;

    if (part1){
        strcat(test, part1);

        length = strlen(part1);

        if ( length && (test[length-1] != *SEP2) && part2 && (*part2 != *SEP2)){
            strcat(test, SEP2);
        }
    }

    if (part2) strcat(test, part2);

    if (file_exists(test)){
        return test;
    }
    free(test);
    return NULL;
}

int file_exists(char * filename){
    struct stat sb;
    return stat(filename, &sb) ? 0:1;
}

off_t file_size(char * filename){
    struct stat sb;
    return stat(filename, &sb) ? 0:sb.st_size;
}

char * file_pathto(char * filename, char * env){
    char *path  = NULL;
    char *test  = NULL;
    char *ptest = NULL;

    // filename is an absolute path
    if (*filename == *SEP2){
        test =  malloc(strlen(filename)+1);
        if (! test) return NULL;
        strcpy(test,filename);
        if (file_exists( test )) return test;
        free(test);
        return NULL;
    }

#ifdef __WIN32__
    // TODO: this will only handle vars up to 512
    char buffer[512];
    if ( ! GetEnvironmentVariable(env,buffer,512) ) return NULL;
    path = buffer;
#else
    path = getenv(env);
    if (! path) return NULL;
#endif

    ptest = test = malloc( strlen(path) + strlen(filename)+20);
    if (! test) return NULL;

    while (*path){
        ptest=test;
        while( (*path) && (*path != SEP1)  ){
            *ptest++ = *path++;
        }

        if ( (ptest != test ) && (*(ptest-1) != *SEP2) && (*filename != *SEP2)){
            *ptest++ = *SEP2;
        }

        *ptest = 0;
        strcat(ptest, filename);
        if (file_exists(test)) return test;
        if (*path == SEP1) path++;
    }
    free(test);
    return  NULL;
}


