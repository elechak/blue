/*
quiz - an automated testing tool
Copyright (C) 2007  Erik R Lechak

email: erik@leckak.info
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



char * result(char * command){
    char buffer[1024];
    FILE * fp                = NULL;
    static char * data       = NULL;
    static size_t capacity   = 0;

    size_t chars_read = 0;
    size_t tot_chars_read = 0;

    #ifdef __WIN32__
        snprintf(buffer, 1024, "cmd /c %s >tempdata.dat", command);
    #else 
        snprintf(buffer, 1024, "%s >tempdata.dat", command);
    #endif
    
    //printf("executing [%s]\n", buffer);
    system(buffer);
    
    fp = fopen("tempdata.dat", "r");
    if ( ! fp){
        printf("could not open temporary data file\n");
        exit(1);
    }
    
    
    
    while(1){
        if ( tot_chars_read  >=  capacity) data = realloc(data, capacity += 1000 );
        chars_read = fread(data+tot_chars_read , 1 ,1000, fp);
        //printf("\nCHARS %i\n", chars_read);
        if (! chars_read) break;
        tot_chars_read += chars_read;
        
    }

    
    data = realloc(data, capacity + 5 );
    
    data[tot_chars_read] = '\n';
    data[tot_chars_read+1] = 0;
    

    fclose(fp);
    return data;
}




char * getCorrect(FILE * fp){
    static char * data       = NULL;
    static size_t capacity   = 0;

    char line[1000];
    size_t chars_read = 0;
    size_t tot_chars_read = 0;

    if (! data){
        data =malloc(1000);
        capacity =1000;
    }


    *data =0;

    while(( fgets(line, 1000,fp) )){

        if ((memcmp(line, "---", 3)==0)) break;

        chars_read = strlen(line);
        tot_chars_read +=chars_read;

        if ( tot_chars_read + 100 >= capacity)
            data = realloc(data, capacity += 1000 );

        strcat(data,line);
    }

    return data;
}



int main(int argc, char **argv){
    FILE * fp    =  NULL;
    char * data  =  NULL;
    char * correct  =  NULL;
    char command_line[4000];
    char line[4000];

    if (argc != 2){
        printf("quiz file needs to be specified\n");
        exit(1);
    }
    
    fp=fopen(argv[1], "r");
    
    if (! fp) {
        printf("Can not find quiz file %s\n", argv[1]);
        exit(1);
    }


    int failed = 0;
    int c=0;

    while(( fgets(line, 4000,fp) )){
        if (*line == '#'){  // COMMENT
            //printf(line);
        }else if (*line == '!'){ // DISPLAYED COMMENT
            printf(line+1);
        }else if ((memcmp(line, "---", 3) == 0)){ // START TEST
            correct = getCorrect(fp);

            if ((strcmp(data,correct)==0)){
                printf(" passed         %s\n", command_line);
            }else{
                failed++;
                printf(" failed         %s\n", command_line);

                printf("------ Returned -----\n");
                printf(data);
                printf("------ Expected ------\n");
                printf(correct);
                printf("-----------------------\n");
            }
        }else if( strspn(line, " \n\r")!=strlen(line) ){ // COMMAND LINE
            
            line[strlen(line)-1] = 0; // remove newline
            printf("test %-5i  ...  ", ++c);
            strcpy(command_line, line);
            //if (data) free(data);
            data = result(command_line);
        }

    }

    if (failed)
        printf("\nFailed %i tests out of %i\n\n", failed, c);
    else
        printf("\nPassed all %i tests \n\n", c);

    remove("tempdata.dat");
    return 0;
}
