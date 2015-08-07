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


#ifndef _THREADING_
#define _THREADING_


#ifndef REENTRANT
#define REENTRANT
#endif

#include <errno.h>

#ifdef __WIN32__
#include <windows.h>
    typedef HANDLE mutex_t;
    typedef HANDLE * Mutex;
    typedef HANDLE thread_t;
#else
#include <pthread.h>
    typedef pthread_mutex_t mutex_t;
    typedef pthread_mutex_t * Mutex;
    typedef pthread_t  thread_t;
#endif


thread_t run_async(void * function , void * data);
void join_async(thread_t thread);
Mutex mutex_new(void);
void mutex_lock(Mutex mutex);
void mutex_unlock(Mutex mutex);
void mutex_free(Mutex mutex);

void threads_enable();
void threads_disable();

#endif
