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

#include "../global.h"
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <regex.h>
#include "../bstring.h"



EXPORT void init(INITFUNC_ARGS);

static string_t instant_format = NULL;

static NativeType instant_type;
static NativeType duration_type;

/* used for translating month and day names into integer values */
static char *monthNames[]={"january", "february","march","april","may","june", 
                              "july", "august", "september", "october","november", "december"};
    
static char *monthAbbrev[]={"jan", "feb","mar","apr","may","jun", 
                              "jul", "aug", "sep", "oct","nov", "dec"};
    
//~ static char *dayNames[] ={"sunday","monday", "tuesday","wednesday","thursday","friday","saturday"};
//~ static char *dayAbbrev[]={"sun","mon", "tues","wed","thur","fri","sat"};


/* regular expression used to interpret a date and/or time */
static regex_t re_time_1;

static regex_t re_today;
static regex_t re_yesterday;
static regex_t re_tomorrow;

static regex_t re_date_1;
static regex_t re_date_2;
static regex_t re_date_3;
static regex_t re_date_4;

static regex_t re_dur_sec;
static regex_t re_dur_min;
static regex_t re_dur_hour;
static regex_t re_dur_day;
static regex_t re_dur_week;
static regex_t re_dur_mon;
static regex_t re_dur_year;

/* translate seconds (floating point) into a timeval */
inline static struct timeval sectotimeval(double sec){
    struct timeval a;
    double f,i;
    f = modf(sec,&i);
    a.tv_sec = (long)i;
    a.tv_usec = (long)(f*1000000);    
    return a;
}

/* convert timeval to double  */
inline static double timevaltod( struct timeval *t){
    return t->tv_sec + t->tv_usec/1000000.0;
}

/* add secons to timeval */
inline static void timevalAdd(struct timeval * t, double seconds){
    double sec = timevaltod( t );
    sec += seconds;
    *t = sectotimeval( sec );
}


/* translate month name/abbrev into integer */
/* TODO: optimize this */
static int match_month( const string_t s){
    
    int c;
    int i = -1;
    
    for( c=0; c<12; c++){
        if ( memcmp(monthNames[c], s->data, s->length+1) == 0 ){
            i = c;
             goto done;
        }
    }
    
    for( c=0; c<12; c++){
        if ( memcmp(monthAbbrev[c], s->data, s->length+1) == 0 ){
            i = c;
            goto done;
        }
    }
    
    done:

    return i;
}

//~ /* translate day name/abbrev into integer */
//~ /* TODO: optimize this */
//~ static int match_day( const string_t s){

    //~ int c;
    //~ int i = -1;
    
    //~ for( c=0; c<7; c++){
        //~ if ( memcmp(dayNames[c], s->data, s->length+1) == 0 ){
            //~ i = c;
             //~ goto done;
        //~ }
    //~ }
    
    //~ for( c=0; c<7; c++){
        //~ if ( memcmp(dayAbbrev[c], s->data, s->length+1) == 0 ){
            //~ i = c;
            //~ goto done;
        //~ }
    //~ }
    
    //~ done:

    //~ return i;
//~ }


/* INSTANT  TYPE*/


static void  create(Link self){
    struct timeval * t = self->value.vptr =  malloc( sizeof( struct timeval));
    t->tv_sec = 0;
    t->tv_usec = 0;
}

static void destroy(Link self){
    if (self->value.vptr) free(self->value.vptr);
    object_destroy(self);   
}

/* time - factory function that can generates an instant
*         with no arguments it returns the current time (instant)
*/
static NATIVECALL(instant_time){    
    Link instant = object_create( instant_type );
    struct timeval * t = instant->value.vptr;
    
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);  
    
    
    if (argn == 1 ) {
        /* ensure argument is a string type */
        string_t s_orig = object_getString(args[0]);
        if (! s_orig) return exception("TimeFormatError", NULL,NULL);
        
        string_t s = string_toLower(s_orig);
        
        struct tm tm;
        int err;
        string_t temp_string;
        
        regmatch_t match[6];
        
        time_t current_time = time(NULL);
        struct tm current_tm = *(localtime(&current_time));
        
        tm = current_tm;
        
        /* TIME */        
        if (! regexec( &re_time_1 ,s->data,6,match,0)){

            tm.tm_sec  = 0;
                   
            /* HOURS */
            if (match[1].rm_so != -1) {
                temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
                tm.tm_hour = strtol(temp_string->data, NULL, 10);
                free(temp_string);
            }
            
            /* AM PM */
            if (match[5].rm_so != -1) {
                temp_string = string_substr( s, match[5].rm_so, match[5].rm_eo);
                                
                switch( temp_string->data[0] ){
                    case 'p': // PM
                        if (tm.tm_hour != 12) tm.tm_hour += 12;
                        break;

                    case 'a': // AM
                        if (tm.tm_hour == 12) tm.tm_hour = 0;
                        break;
                }
                
                free(temp_string);
            }            
            
            /* MIN */
            if (match[2].rm_so != -1) {
                temp_string = string_substr( s, match[2].rm_so, match[2].rm_eo);
                tm.tm_min = strtol(temp_string->data, NULL, 10);
                free(temp_string);
            }

            /* SEC */
            if (match[4].rm_so != -1){
                temp_string = string_substr( s, match[4].rm_so, match[4].rm_eo);
                tm.tm_sec = strtol(temp_string->data, NULL, 10);
                free(temp_string);
            }
        }
        
        
        /* DATE    25-Dec-2001*/
        if (! regexec( &re_date_1 ,s->data,6,match,0)){  
            
            /* MONTH */
            temp_string = string_substr( s, match[2].rm_so, match[2].rm_eo);
            err = match_month(temp_string);
            if ( err != -1){
                tm.tm_mon = err;
            }
            free(temp_string);
            
            /* DAY */
            temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
            tm.tm_mday = strtol(temp_string->data, NULL, 10);
            free(temp_string);
            
            /* YEAR */
            temp_string = string_substr( s, match[3].rm_so, match[3].rm_eo);
            tm.tm_year = strtol(temp_string->data, NULL, 10)-1900;
            free(temp_string);
            goto done;
        }
        
        /* DATE   2001-12-25 YYYY-MM-DD*/
        if (! regexec( &re_date_2 ,s->data,6,match,0)){  

            /* YEAR */
            temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
            tm.tm_year = strtol(temp_string->data, NULL, 10)-1900;
            free(temp_string);
            
            /* MONTH */
            temp_string = string_substr( s, match[2].rm_so, match[2].rm_eo);
            tm.tm_mon = strtol(temp_string->data, NULL, 10)-1;
            free(temp_string);
            
            /* DAY */
            temp_string = string_substr( s, match[3].rm_so, match[3].rm_eo);
            tm.tm_mday = strtol(temp_string->data, NULL, 10);
            free(temp_string);
            
            goto done;
        }        

        /* DATE   12/25/2001*/
        if (! regexec( &re_date_3 ,s->data,6,match,0)){  
            
            /* MONTH */
            temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
            tm.tm_mon = strtol(temp_string->data, NULL, 10)-1;
            free(temp_string);
            
            /* DAY */
            temp_string = string_substr( s, match[2].rm_so, match[2].rm_eo);
            tm.tm_mday = strtol(temp_string->data, NULL, 10);
            free(temp_string);            
            
            /* YEAR */
            temp_string = string_substr( s, match[3].rm_so, match[3].rm_eo);
            tm.tm_year = strtol(temp_string->data, NULL, 10)-1900;
            free(temp_string);
            
            goto done;
        }         
        
        /* DATE   July 4,1975 */
        if (! regexec( &re_date_4 ,s->data,6,match,0)){  
            
            /* MONTH */
            temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
            err = match_month(temp_string);
            if ( err != -1){
                tm.tm_mon = err;
            }
            free(temp_string);            
                        
            /* DAY */
            temp_string = string_substr( s, match[2].rm_so, match[2].rm_eo);
            tm.tm_mday = strtol(temp_string->data, NULL, 10);
            free(temp_string);            
            
            /* YEAR */
            temp_string = string_substr( s, match[3].rm_so, match[3].rm_eo);
            tm.tm_year = strtol(temp_string->data, NULL, 10)-1900;
            free(temp_string);
            
            goto done;
        }   
        
        done:
        free(s);
        t->tv_sec = (long) mktime( &tm);    
    }else{
        gettimeofday( t, NULL);
    }
    
    return instant;
}


/* set the default print format for time */
static NATIVECALL(time_format){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    if (argn > 0){
        string_t s = object_getString(args[0]);
        if (s){
            free(instant_format);
            instant_format = string_dup(s);    
        }
    }
    return create_string_str( instant_format );
}

static string_t instant_asString(Link self){
    char str[128];
    struct timeval * t = self->value.vptr;
    strftime(str, 128, instant_format->data, localtime( (time_t * )&t->tv_sec ) );
    return string_new(str);
}

static NATIVECALL(instant_epoch){ 
    struct timeval * t = This->value.vptr; 
    return create_numberd(t->tv_sec + t->tv_usec/1000000.0);
}

static Link instant_op_plus(Link self, Link other){
    if (other->type != duration_type) return exception("ArgTypeError", NULL, NULL);
    struct timeval * a = self->value.vptr;
    struct timeval * b = other->value.vptr;

    Link link = object_create( instant_type );
    struct timeval * c = link->value.vptr;
    
    c->tv_sec = a->tv_sec + b->tv_sec;
    c->tv_usec = a->tv_usec + b->tv_usec;
    
    if (c->tv_usec > 1000000){
        c->tv_sec ++;
        c->tv_usec -=1000000;
    }else if (c->tv_usec < -1000000){
        c->tv_sec --;
        c->tv_usec +=1000000;
    }
    
    return link;    
}

static Link instant_op_minus(Link self, Link other){
    if (other->type != instant_type) return exception("ArgTypeError", NULL, NULL);
    
    struct timeval * a = self->value.vptr;
    struct timeval * b = other->value.vptr;

    Link link = object_create( duration_type );
    struct timeval * c = link->value.vptr;
    
    c->tv_sec = a->tv_sec - b->tv_sec;
    c->tv_usec = a->tv_usec - b->tv_usec;
    
    if (c->tv_usec > 1000000){
        c->tv_sec ++;
        c->tv_usec -=1000000;
    }else if (c->tv_usec < -1000000){
        c->tv_sec --;
        c->tv_usec +=1000000;
    }
    
    return link;
}


static int instant_compare(Link self, Link other){
    if (other->type != instant_type) return 2;
    
    struct timeval * a = self->value.vptr;
    struct timeval * b = other->value.vptr;    
    
    long c = a->tv_sec - b->tv_sec;
    
    if (c ==0){
        c = a->tv_usec - b->tv_usec;
        
        if (c ==0) return 0;
    }
    
    return c>0  ? 1 :-1;
}

static NATIVECALL(instant_printFormat){ 
    
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    char * format = NULL;
    if (argn == 1){
        string_t s = object_getString(args[0]);
        if (s){
            format = s->data;    
        }     
    }
    
    if (! format) format = "%D";
    
    char str[128];
    
    struct timeval * t = This->value.vptr;
    strftime(str, 128, format, localtime( (time_t * )&t->tv_sec ) );
    
    return create_string(str);
}




/* DURATION */

static NATIVECALL(duration){    
    Link instant = object_create( duration_type );
    struct timeval * t = instant->value.vptr;
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    t->tv_sec   = 0;
    t->tv_usec  = 0;
    
    number_t n = 0.0;
    
    string_t s_orig;

    double i;
    double f;
    
    if (argn){
        if (args[0]->type == Global->number_type){
            n = args[0]->type->asNumber(args[0]);
            f = modf(n,&i);
            f *= 1000000;
            t->tv_sec = (long) i;
            t->tv_usec= (long) f;
        }else if ( (s_orig = object_getString(args[0])) ) {
            string_t temp_string;
            regmatch_t match[2];
            string_t s = string_toLower(s_orig);
            
            /* SECONDS */
            if (! regexec( &re_dur_sec ,s->data,3,match,0)){
                temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
                timevalAdd( t, strtod(temp_string->data, NULL) );
                free(temp_string);
            }
            
            /* MINUTES */
            if (! regexec( &re_dur_min ,s->data,3,match,0)){
                temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
                timevalAdd( t, strtod(temp_string->data, NULL) * 60 );
                free(temp_string);
            }            
            
            /* HOURS */
            if (! regexec( &re_dur_hour ,s->data,3,match,0)){
                temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
                timevalAdd( t, strtod(temp_string->data, NULL) * 60 * 60);
                free(temp_string);
            }

            /* DAYS */
            if (! regexec( &re_dur_day ,s->data,3,match,0)){
                temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
                timevalAdd( t, strtod(temp_string->data, NULL) * 60 * 60 * 24);
                free(temp_string);
            }            
            
            /* WEEKS */
            if (! regexec( &re_dur_week ,s->data,3,match,0)){
                temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
                timevalAdd( t, strtod(temp_string->data, NULL) * 60 * 60 * 24 * 7);
                free(temp_string);
            }
            
            /* MONTHS */
            if (! regexec( &re_dur_mon ,s->data,3,match,0)){
                temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
                timevalAdd( t, strtod(temp_string->data, NULL) * 60 * 60 * 24 * 7 * 4);
                free(temp_string);
            }            
            
            /* YEARS */
            if (! regexec( &re_dur_year ,s->data,3,match,0)){
                temp_string = string_substr( s, match[1].rm_so, match[1].rm_eo);
                timevalAdd( t, strtod(temp_string->data, NULL) * 60 * 60 * 24 * 365);
                free(temp_string);
            }            
            
            free(s);
        }
    }
    
    return instant;
}


static string_t duration_asString(Link self){
    char str[128];
    struct timeval * t = self->value.vptr;
    snprintf(str, 128, "%f seconds",t->tv_sec + t->tv_usec/1000000.0);
    return string_new(str);
}


static Link duration_op_plus(Link self, Link other){
    struct timeval * a = self->value.vptr;
    struct timeval * b = other->value.vptr;

    Link link = NULL;
    
    if (other->type == duration_type){
        link = object_create( duration_type );    
    }else if (other->type == instant_type){
        link = object_create( instant_type );    
    }else {
        return exception("ArgTypeError", NULL, NULL);
    }
    
    struct timeval * c = link->value.vptr;
    
    c->tv_sec = a->tv_sec + b->tv_sec;
    c->tv_usec = a->tv_usec + b->tv_usec;
    
    if (c->tv_usec > 1000000){
        c->tv_sec ++;
        c->tv_usec -=1000000;
    }else if (c->tv_usec < -1000000){
        c->tv_sec --;
        c->tv_usec +=1000000;
    }
    
    return link;    
}

static Link duration_op_minus(Link self, Link other){
    struct timeval * a = self->value.vptr;
    struct timeval * b = other->value.vptr;

    Link link = NULL;
    
    if (other->type == duration_type){
        link = object_create( duration_type );    
    }else if (other->type == instant_type){
        link = object_create( instant_type );    
    }else {
        return exception("ArgTypeError", NULL, NULL);
    }
    
    struct timeval * c = link->value.vptr;
    
    c->tv_sec = a->tv_sec - b->tv_sec;
    c->tv_usec = a->tv_usec - b->tv_usec;
    
    if (c->tv_usec > 1000000){
        c->tv_sec ++;
        c->tv_usec -=1000000;
    }else if (c->tv_usec < -1000000){
        c->tv_sec --;
        c->tv_usec +=1000000;
    }
    
    return link;    
}



static Link duration_op_multiply(Link self, Link other){
    if (other->type != Global->number_type) return exception("DurationMultiplyError", NULL, NULL);
    struct timeval * a = self->value.vptr;

    double t = (a->tv_sec + a->tv_usec/1000000.0) * other->value.number;

    Link link = object_create( duration_type );
    a = link->value.vptr;
    
    double f,i;
    f = modf(t,&i);
    a->tv_sec = (long)i;
    a->tv_usec = (long)(f*1000000);
    
    return link;
}


static Link duration_op_divide(Link self, Link other){
    if (other->type != Global->number_type) return exception("DurationDivideError", NULL, NULL);
    struct timeval * a = self->value.vptr;

    double t = (a->tv_sec + a->tv_usec/1000000.0) / other->value.number;
    
    Link link = object_create( duration_type );
    a = link->value.vptr;
    
    double f,i;
    f = modf(t,&i);
    a->tv_sec = (long)i;
    a->tv_usec = (long)(f*1000000);
    
    return link;
}


static int duration_compare(Link self, Link other){
    if (other->type != duration_type) return 2;
    
    struct timeval * a = self->value.vptr;
    struct timeval * b = other->value.vptr;    
    
    long c = a->tv_sec - b->tv_sec;
    
    if (c ==0){
        c = a->tv_usec - b->tv_usec;
        
        if (c ==0) return 0;
    }
    
    return c>0  ? 1 :-1;
}



void init(INITFUNC_ARGS){
    
    // regex that parses out the time portion
     regcomp(&re_time_1, "([[:digit:]]+):([[:digit:]]+)(:([[:digit:]]+))? {0,1}(am|pm){0,1}",REG_EXTENDED);
    
     regcomp(&re_today, "today",REG_EXTENDED); 
     regcomp(&re_yesterday, "yesterday",REG_EXTENDED); 
     regcomp(&re_tomorrow, "tomorrow",REG_EXTENDED); 
    
    // regex that parses out the date portion
     regcomp(&re_date_1,"([[:digit:]]+)[-\\/](\\w{3,})[-\\/]([[:digit:]]+)",REG_EXTENDED);          //  25-Dec-2001
     regcomp(&re_date_2,"([[:digit:]]{4})[-\\/]([[:digit:]]+)[-\\/]([[:digit:]]+)",REG_EXTENDED);           //  2001-12-25 YYYY-MM-DD
     regcomp(&re_date_3,"([[:digit:]]{1,2})[-\\/]([[:digit:]]{1,2})[-\\/]([[:digit:]]{2,4})",REG_EXTENDED); //  12/25/2001
     regcomp(&re_date_4,"(\\w{3,}) ([[:digit:]]+), *([[:digit:]]+)",REG_EXTENDED);                //  July 4,1975

    // regex that parses out any time or date adjustments
     regcomp(&re_dur_sec,"([[:digit:]\\.]+)\\s{0,1}seconds?",REG_EXTENDED);
     regcomp(&re_dur_min,"([[:digit:]\\.]+)\\s{0,1}minutes?",REG_EXTENDED);
     regcomp(&re_dur_hour,"([[:digit:]\\.]+)\\s{0,1}hours?",REG_EXTENDED);
     regcomp(&re_dur_day,"([[:digit:]\\.]+)\\s{0,1}days?",REG_EXTENDED);
     regcomp(&re_dur_week,"([[:digit:]\\.]+)\\s{0,1}weeks?",REG_EXTENDED);
     regcomp(&re_dur_mon,"([[:digit:]\\.]+)\\s{0,1}months?",REG_EXTENDED);
     regcomp(&re_dur_year,"([[:digit:]\\.]+)\\s{0,1}years?",REG_EXTENDED);  
    
    
    instant_format = string_new("%A %D %r");

    instant_type = newNative();
        instant_type->create   = create;
        instant_type->destroy  = destroy;
        instant_type->asString = instant_asString;
        instant_type->op_minus = instant_op_minus;
        instant_type->op_plus  = instant_op_plus;
        instant_type->compare  = instant_compare;
    
        addNativeCall(instant_type, "format", instant_printFormat);
        addNativeCall(instant_type, "epoch", instant_epoch);
        addNativeCall(instant_type, "print", universal_print);
    
    duration_type = newNative();
        duration_type->create       = create;
        duration_type->destroy      = destroy;
        duration_type->asString     = duration_asString;
        duration_type->op_minus     = duration_op_minus;
        duration_type->op_plus      = duration_op_plus;
        duration_type->op_multiply  = duration_op_multiply;
        duration_type->op_divide    = duration_op_divide;
        duration_type->compare      = duration_compare;
        
        addNativeCall(duration_type, "print", universal_print);
    
    
    addCFunc(Module, "time", instant_time);
    addCFunc(Module, "duration", duration);
    addCFunc(Module, "format", time_format);
}





