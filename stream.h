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

#ifndef _STREAM
#define _STREAM

// --------------------------- COMMON ---------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h> // close
#include <fcntl.h> // O_RDWR
#include "bstring.h"

// ----------------------- WINDOWS ---------------------------
#ifdef __WIN32__
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
// ----------------------- LINUX ---------------------------------
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h> // gethostbyname
#endif

struct StreamServer{
    int port;
    int server; // file descriptor for server
};
typedef struct StreamServer * StreamServer;


struct Stream{
    size_t capacity;
    size_t write_offset;
    size_t read_offset;
    unsigned char *buffer;

    char type;

    int pid; // holds the pid for the child process for exec type

    int input_descriptor;
    int output_descriptor;
#ifdef __WIN32__
    HANDLE input_handle;
    HANDLE output_handle;
#endif
};
typedef struct Stream * Stream;


//CONSTRUCTORS
Stream stream_open_mem(); // memory type
Stream stream_open_socket(char * server_name); // connects to server via socket
Stream stream_open_accept(StreamServer ss); // waits for connections (a server)
Stream stream_open_file(char * filename, char * type, mode_t mode); // file stream
Stream stream_open_shell(); // like system but returns stdout -- blocks
Stream stream_open_exec(char * command); // like popen - non-blocking
Stream stream_open_stdio(); // stream connected to stdin stdout
Stream stream_open_const_cstr(const char * cstring);

//DESTRUCTOR
void stream_close(Stream self);
void stream_clear(Stream self); // resets read and write offsets

//WRITE
void stream_write(Stream self, string_t string);
void stream_writef(Stream self, const char * format, ...);

//READ
int stream_ready(Stream self);
string_t stream_read(Stream self, size_t amount);
string_t stream_readbreak(Stream self, string_t little);

void streamserver_close(StreamServer self);
StreamServer stream_server(int port);


#endif
