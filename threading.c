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
#include <stdlib.h>
#include "threading.h"
#include <stdio.h>

static int threads_enabled = 1;


void threads_enable(){
    threads_enabled = 1;
}

void threads_disable(){
    threads_enabled = 0;
}



Mutex mutex_new(){
    Mutex mutex = malloc(sizeof(mutex_t));
    if (! mutex) return NULL;
#ifdef __WIN32__
    *mutex = CreateMutex(NULL,FALSE,NULL);
    if (! *mutex ){
        free( mutex );
        return NULL;
    }
#else
    if ( pthread_mutex_init( mutex,NULL)){
        free( mutex );
        return NULL;
    }
#endif
    return mutex;
}


void mutex_lock(Mutex mutex){
    if (! threads_enabled ) return;
    #ifdef __WIN32__
        if (WaitForSingleObject(*mutex, INFINITE)== WAIT_FAILED ){
            printf("mutex locking error, %d\n", GetLastError());
            fflush(stdout);
            exit(1);
            //TODO: do something good here
        }
    #else
        pthread_mutex_lock(mutex);
    #endif
}


void mutex_unlock(Mutex mutex){
    if (! threads_enabled ) return;
    #ifdef __WIN32__
        if (! ReleaseMutex(*mutex) ){
            printf("mutex unlocking error -->%d\n", GetLastError());
            fflush(stdout);
            exit(1);
            //TODO: do something good here
        }
    #else
        pthread_mutex_unlock(mutex);
    #endif
}


void mutex_free(Mutex mutex){

#ifdef __WIN32__
    CloseHandle(*mutex);
#else
    pthread_mutex_destroy(mutex);
#endif
    free(mutex);
}



thread_t run_async(void * function , void * data){
    if (! threads_enabled ){
        ((void (*)(void *))function)(data);
        return 0;
    }
    
    thread_t thread_handle;
    #ifdef __WIN32__    
        DWORD thread_id;
        thread_handle = CreateThread(NULL,0,function,data,0,&(thread_id));
    #else
        pthread_create(&(thread_handle),NULL,function,data);
        //pthread_detach(thread_handle);
    #endif
    return thread_handle;
}


void join_async(thread_t thread){
    if (! threads_enabled ) return;
    #ifdef __WIN32__   
        
        if (WaitForSingleObject(thread, INFINITE)== WAIT_FAILED ){
            //TODO: do something good here
            exit(1);
        }
        
    #else
        int error = pthread_join(thread,NULL);
        
        if (error){
            switch (error){
                case EINVAL:
                    fprintf(stderr,"join_async error - non joinable thread\n");fflush(stderr);
                    break;
                
                case ESRCH:
                    fprintf(stderr,"join_async error - no thread found\n");fflush(stderr);
                    break;

                case EDEADLK:
                    fprintf(stderr,"join_async error - deadlock detected\n");fflush(stderr);
                    break;
                
                default:
                    fprintf(stderr,"join_async error - unknown error\n");fflush(stderr);
                    break;
            }
        }
    #endif
}





