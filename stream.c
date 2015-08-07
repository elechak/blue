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
#include "stream.h"
#include "files.h"
#include <errno.h>


#define SOURCE_INIT_SIZE  1024
#define SOURCE_READ_SIZE  SOURCE_INIT_SIZE-1
#define TOKEN_INIT_SIZE  64

#ifdef __WIN32__
    static WSADATA wsaData;
    static int wsaStarted = 0;
#endif


/* ensure that stream has sufficient memory to hold SIZE bytes */
static inline void stream_alloc(Stream self, size_t size){
    size_t remainder;

    /* how much write space remains */
    remainder = self->capacity - self->write_offset;

    if (remainder < size){
        /* Need to do something to get more space */

        /* Pack the contents */
            // this is the amount of unread buffered data
            remainder = self->write_offset - self->read_offset;

            // there is some data left and there is free space at the beginning
            if ( ( remainder ) && (self->read_offset)){
                //shift all the data to the left to get rid of the free space
                memmove(self->buffer, self->buffer + self->read_offset, remainder);
                self->read_offset = 0;
                self->write_offset = remainder;
            }

        /* Grow if necessary */
            // how much space remains
            remainder = self->capacity - self->write_offset;

            // if there is not enough room in the buffer memory we need to allocate more
            if ( remainder < size ){
                self->capacity += size-remainder;
                self->buffer = realloc(self->buffer, self->capacity);
                remainder = self->capacity - self->write_offset;
            }
    }
}


#ifdef __WIN32__
void shell_write(Stream self, char * command){

    static const char shell[] = "cmd /c ";
    
    char * full_command = malloc( strlen(command) + 8);
    
    memcpy(full_command, shell, 7);
    memcpy(full_command+7, command, strlen(command)+1); 
    
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    
    ZeroMemory( &pi, sizeof(pi) );
    ZeroMemory( &si, sizeof(si) );
    
    HANDLE childout, input_handle;
    CreatePipe(&input_handle, &childout, NULL, 0);
    SetHandleInformation( childout, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    si.cb = sizeof(STARTUPINFO);
    si.hStdOutput = childout;
    si.dwFlags |= STARTF_USESTDHANDLES;

    if  (! CreateProcess(NULL,
          full_command,       // command line
          NULL,          // security attributes
          NULL,          // thread security attributes
          TRUE,          // handles are inherited
          CREATE_NO_WINDOW,// creation flags
          NULL,          // use parent's environment
          NULL,          // use parent's current directory
          &si,  // STARTUPINFO pointer
          &pi)  // receives PROCESS_INFORMATION
    ){
        free(full_command);
        CloseHandle(childout);
        return;
    }

    free(full_command);
    CloseHandle(childout);
    WaitForSingleObject( pi.hProcess, INFINITE );

    int fd = _open_osfhandle((long)input_handle,_O_RDONLY);
        
    int chars_read;
    while ( 1 ){
        stream_alloc(self,1024);
        chars_read = read(fd ,self->buffer + self->write_offset, 1024);
        self->write_offset +=chars_read;        
        if (chars_read < 1024)break;
    }    
    
    close(fd);
    CloseHandle(input_handle);
    
}

#else
void shell_write(Stream self, char * command){

    static char shell[]  = "sh";
    static char arg0[]   = "-c";
    char * args[4] = {shell, arg0, command, NULL};

    int pid;
    int out_pipe[2];

    pipe(out_pipe); // pipe used to read from child
    pid = fork();

    switch(pid){
        case -1: // ERROR
            return;

        case 0: // CHILD
            close(1); //close stdout
            dup(out_pipe[1]);
            close(out_pipe[0]);
            close(out_pipe[1]);

            execvp(args[0],args);
        
            exit(1); //an error occured
            break;

        default: // PARENT
			close(out_pipe[1]); //close write end of out_pipe
            waitpid(pid, 0, 0);
    }

    int chars_read;
    while ( 1 ){
        stream_alloc(self,1024);
        chars_read = read(out_pipe[0] ,self->buffer + self->write_offset, 1024);
        self->write_offset +=chars_read;        
        if (chars_read < 1024)break;
    }
    close( out_pipe[0]);
}
#endif




// clear the stream's buffer: resets read and write offsets
void stream_clear(Stream self){
    self->read_offset =0;
    self->write_offset =0;    
}


int stream_ready(Stream self){
    fd_set reads;
    int ret;

    
    if ((self->type =='m') || (self->type =='h')) return 1;
    
#ifdef __WIN32__
    long available;
    //DWORD dw;
    switch (self->type){
        case 'x': // EXEC
            if (PeekNamedPipe( self->input_handle,NULL,0,NULL,&available,NULL)){
                return available;
            }else{
                printf("Windows Error <%i>\n", (int)GetLastError() );
                return 1;
            }
            break;

        case 'f': // FILE        
            return 1;
            break;

        case 's': // SOCKET
            if (self->input_descriptor == -1) return 0;
            FD_ZERO(&reads);
            FD_SET(self->input_descriptor, &reads);
            ret = select(self->input_descriptor + 1, &reads, NULL, NULL, NULL);
            if (ret > 0) return 1;
            break;
    }

#else
    // LINUX
    switch (self->type){
        case 'x': // EXEC
        case 's': // SOCKET
        case 'f': // FILE
            if (self->input_descriptor == -1) return 0;
            FD_ZERO(&reads);
            FD_SET(self->input_descriptor, &reads);
            ret = select(self->input_descriptor + 1, &reads, NULL, NULL, NULL);
            if (ret > 0) return 1;
            break;
    }

#endif

    return 0;
}


string_t stream_read(Stream self, size_t amount){

    size_t chars_read   = 0;
    size_t remainder    = 0;

    /* calculate the amount of data current buffered in the stream */
    remainder = self->write_offset - self->read_offset;

    if ((remainder == 0) && (self->type == 's') && ( self->input_descriptor == -1)) return NULL;


    /* if the amount of data buffered in the stream is less than the requested amount we need to read again */
    if ( (remainder < amount) &&
         (    (self->type == 'f')||   // file
              (self->type == 'i')||   // stdio
              (self->type == 's')||   // socket              
              (self->type == 'x')     // process
         ) && (self->input_descriptor != -1) // input descriptor is still active
       ){

        /* make sure the stream has enough memory */
        stream_alloc(self,amount);

        if (self->type == 's'){
            /* read socket */
            chars_read = recv(self->input_descriptor,self->buffer + self->write_offset, amount,0 );
            //printf("chars read = %i  s\n", chars_read);
            if (chars_read ==0 ){
#ifdef __WIN32__
            if (self->input_descriptor != -1) closesocket(self->input_descriptor);
#else
            if (self->input_descriptor != -1){
                shutdown(self->input_descriptor, SHUT_RDWR);
                close(self->input_descriptor);
            }
#endif
            self->input_descriptor = -1;
            }
        }else{
            /* read from file type descriptor */
            //fprintf(stderr,"READING\n");
            chars_read = read(self->input_descriptor,self->buffer + self->write_offset, amount);
            //fprintf(stderr,"READ %i characters\n", chars_read);
        }

        if ((chars_read == 0) && (self->write_offset - self->read_offset == 0 )) return NULL;
        self->write_offset +=chars_read;
        remainder = self->write_offset - self->read_offset;
    }

    /* limit the amount of data that can be returned to amount of data in the buffer */
    if (amount > remainder){
        amount = remainder; // can only return the amount of data in buffer
    }
    string_t ret = string_new_ls(amount, (char *)self->buffer + self->read_offset);
    self->read_offset += amount;
    return ret;
}



string_t stream_readbreak(Stream self, string_t little){
    size_t chars_read      = 0;
    size_t remainder       = 0;
    size_t end             = 0;
    size_t match           = 0;
    size_t remember_offset = self->read_offset;
    string_t ret = NULL;

    const char firstchar = little->data[0]; // first character in string to match

    while( ! match){
        remainder = self->write_offset - remember_offset;  // how many unread bytes remain in buffer

        /* try to find a match */
            if (remainder >= little->length){     // there are enough bytes remaining to contain the search string
                end = self->write_offset - little->length+1;

                /* step through the bytes remaining looking for match */
                for (;remember_offset < end; remember_offset++){
                    if (( *(self->buffer+remember_offset) == firstchar) && (memcmp(self->buffer+remember_offset, little->data, little->length) ==0 )){
                        match = remember_offset+little->length; // match remebers the location of the end of the search string in the buffer
                        break;
                    }
                }

                if (match) break;
            }

        /* read more data , did not find match*/
            /* make sure that it is posible to read more data */
            if  (!( (self->type == 'f')||(self->type == 's')||(self->type == 'x')||(self->type == 'i'))) break;
            if  (self->input_descriptor == -1) break;

            end = self->read_offset; // need to remember read_offset because stream_alloc will set it to 0
            remember_offset -= self->read_offset; // turn remember_offset into an offset from  read_offset
            stream_alloc(self, 1024   );
            remember_offset +=self->read_offset; // convert remember_offset back to absolute offset

            if (self->type == 's')
                chars_read = recv(self->input_descriptor,self->buffer + self->write_offset, 1024,0 );
            else
                chars_read = read(self->input_descriptor,self->buffer + self->write_offset, 1024);
            
            //printf("\n%i{\n%.*s\n}\n", chars_read,chars_read,self->buffer + self->write_offset );

            if (chars_read == 0) break;

            self->write_offset +=chars_read;
    }

    if (! match) return NULL;

    ret = string_new_ls(match - self->read_offset, (char *) self->buffer + self->read_offset);
    self->read_offset = match;

    return ret;
}


void stream_write(Stream self, string_t string){
    size_t size = string->length;
    char * data = string->data;

    switch (self->type){
        case 'f': // FILE
        case 'x': // EXEC
        case 'i': // STDIO        
            if (self->output_descriptor == -1) break;
            write(self->output_descriptor, data, size) ;
            break;
        case 's': // SOCKET
            if (self->output_descriptor == -1) break;
            send(self->output_descriptor ,data, size,0 );
            break;
        case 'm': // MEMORY
            /* make sure there is enough room for the data */
            stream_alloc(self,size);

            /* Write the data */
            memcpy(self->buffer+self->write_offset, data, size);
            self->write_offset += size;
            break;
        case 'h': // SHELL
            shell_write( self, data);
            break;
    }
}


void stream_writef(Stream self, const char * format, ...){

    /* find length */
    int length = -1;
    char * buffer = NULL;
    int buffer_size = 0;
    va_list va;
    
    va_start(va, format);
    length = vsnprintf(NULL,0,format,va);
    va_end(va);
    
    while( length < 0 ){
        buffer_size +=1000;
        buffer = malloc( buffer_size );
        va_start(va, format);
        length = vsnprintf(buffer,1000,format,va);
        va_end(va);
        free(buffer);
    }       
    
    string_t s = string_new_ls( length, NULL);
    
    va_start(va, format);
    length = vsnprintf(s->data,length+1,format,va);
    va_end(va);

    stream_write( self, s);
    free(s);
}


static Stream stream_new(){
    Stream self = malloc(sizeof(struct Stream));
    if (! self) return NULL;

    self->input_descriptor  = -1;
    self->output_descriptor = -1;

    self->buffer = NULL;
    self->capacity = 0;

    self->read_offset =0;
    self->write_offset =0;

    self->type = 0;

    return self;
}


Stream stream_open_shell(){
    Stream self = stream_new();
    self->type = 'h';
    return self;    
}

Stream stream_open_mem(){
    Stream self = stream_new();
    self->type = 'm';
    return self;
}


Stream stream_open_const_cstr(const char * cstring){
    if (! cstring) return NULL;
    Stream self = stream_new();
    if (! self) return NULL;
    self->buffer = (unsigned char *)cstring;
    self->capacity = strlen(cstring);
    self->type = 'c';
    return self;
}

Stream stream_open_stdio(){

    Stream self = stream_new();
    if (! self) return NULL;

    self->type = 'i';

    self->output_descriptor = fileno(stdout);
    self->input_descriptor  = fileno(stdin);
    return self;
}


Stream stream_open_socket(char * server_name){

    struct sockaddr_in  address;
    struct hostent * server;
    int sd, port,status;
    char hostname[64];

#ifdef __WIN32__
    // Initialize winsock sockets
    if ( ! wsaStarted){
        if ( (WSAStartup(0x0101, &wsaData) != NO_ERROR) ){
            printf("WSAStartup failed\n");
            exit(1);
        }else{
            wsaStarted =1;
        }
    }
#endif

    /* create generic stream */
    Stream self = stream_new();
    if (! self) return NULL;
    self->type = 's';

    status = strcspn(server_name,":");
    strncpy(hostname, server_name,status);
    hostname[status] = '\0';
    port =  atoi( server_name+1+status );

    sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd < 0){
            fprintf(stderr,"ERROR opening socket");
            return NULL;
        }

    server = gethostbyname(hostname);
        if (server == NULL) {
            fprintf(stderr,"no host\n");
            return NULL;
        }

        address.sin_family = AF_INET;
        address.sin_port = htons(port);
        address.sin_addr.s_addr = htonl(INADDR_ANY);
        memcpy(&address.sin_addr.s_addr,server->h_addr,server->h_length );

        status = connect(sd,(struct sockaddr *)&address,sizeof(address));

        if (status < 0 ) return NULL;

        self->output_descriptor = self->input_descriptor = sd;

        return self;
}



StreamServer stream_server(int port){

#ifdef __WIN32__
    // Initialize winsock sockets
    if ( ! wsaStarted){
        if ( (WSAStartup(0x0101, &wsaData) != NO_ERROR) ){
            printf("WSAStartup failed\n");
            exit(1);
        }else{
            wsaStarted =1;
        }
    }
#endif

    StreamServer self = malloc(sizeof(self));

    struct sockaddr_in  server;
    int server_descriptor,bind_result;

    server_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (server_descriptor < 0 ){
        fprintf(stderr, "Can not open socket on port %i [%i]\n", port, server_descriptor);
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    bind_result = bind(server_descriptor,(struct sockaddr *)&server,sizeof(struct sockaddr_in));
    if (bind_result){
        return NULL;
        //fprintf(stderr,"Bind problem %i\n", port);
    }

    //fprintf(stderr,"Listening on port %i\n", port);
    listen(server_descriptor,5);

    self->port  = port;
    self->server = server_descriptor;
    return self;
}

void streamserver_close(StreamServer self){
    close(self->server);
    self->server = -1;
    self->port = -1;
}


// Server Socket --- this will block
Stream stream_open_accept(StreamServer ss){

    int server_descriptor = ss->server;

    //struct sockaddr_in  client;
    //int client_length = sizeof(struct sockaddr);

#ifdef __WIN32__
    // Initialize winsock sockets
    if ( ! wsaStarted){
        if ( (WSAStartup(0x0101, &wsaData) != NO_ERROR) ){
            printf("WSAStartup failed\n");
            exit(1);
        }else{
            wsaStarted =1;
        }
    }
#endif

    Stream self = stream_new();
    if (! self) return NULL;
    self->type = 's';
    self->output_descriptor = self->input_descriptor = accept(server_descriptor, NULL,NULL);
    return self;
}


// open a file as a stream for reading and writing
Stream stream_open_file(char * filename, char * type, mode_t mode){
    unsigned long flags = 0;

    Stream self = stream_new();

    if (! self) return NULL;

    self->type = 'f';

    if (strchr(type, 'c')){  // create if it does not exist
        self->input_descriptor = open(filename, O_RDONLY | O_CREAT, mode);
        close(self->input_descriptor);
        self->input_descriptor = -1;
    }

    if (strchr(type, 'a')){  // append
        flags |= O_APPEND;
        flags |= O_WRONLY;
        self->output_descriptor =  open(filename, flags, mode);

    } else if (strchr(type, 'w')){  // write
        flags |= O_CREAT;
        flags |= O_TRUNC;
        flags |= O_WRONLY;
        self->output_descriptor =  open(filename, flags, mode);
    }

    if (strchr(type, 'r')){
        self->input_descriptor = open(filename, O_RDONLY, mode);
    }    
    

    return self;
}


#ifdef __WIN32__

Stream stream_open_exec(char * command){

    static const char shell[] = "cmd /c ";
    char * full_command = malloc( strlen(command) + 8);
    memcpy(full_command, shell, 7);
    memcpy(full_command+7, command, strlen(command)+1);     
    
    Stream self = stream_new();
    if (! self) return NULL;
    self->type = 'x';

    PROCESS_INFORMATION pi;
    ZeroMemory( &pi, sizeof(pi) );

    HANDLE childerr;
    childerr = GetStdHandle(STD_ERROR_HANDLE);
    SetHandleInformation( childerr, HANDLE_FLAG_INHERIT, 0);

    HANDLE childout, input_handle;
    CreatePipe(&input_handle, &childout, NULL, 0);
    SetHandleInformation( childout, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    HANDLE childin, output_handle;
    CreatePipe(&childin, &output_handle, NULL, 0);
    SetHandleInformation( childin, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);

    STARTUPINFO si;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(STARTUPINFO);
    si.hStdOutput = childout;
    si.hStdInput = childin;
    si.dwFlags |= STARTF_USESTDHANDLES;

    if  (! CreateProcess(NULL,
          full_command,       // command line
          NULL,          // security attributes
          NULL,          // thread security attributes
          TRUE,          // handles are inherited
          CREATE_NO_WINDOW,// creation flags
          NULL,          // use parent's environment
          NULL,          // use parent's current directory
          &si,  // STARTUPINFO pointer
          &pi)  // receives PROCESS_INFORMATION
    ){
        printf("CreateProcessError <%i>\n", (int)GetLastError());
    }

    free(full_command);
    CloseHandle(childout);
    CloseHandle(childin);

    self->input_handle = input_handle;
    self->output_handle = output_handle;
    self->input_descriptor = _open_osfhandle((long)input_handle,_O_RDONLY);
    self->output_descriptor = _open_osfhandle((long)output_handle,_O_APPEND);

    return self;
}


#else
Stream stream_open_exec(char * command){
    // All of this to generate the array of strings for execvp
    static char shell[]  = "sh";
    static char arg0[]   = "-c";
    char * args[4] = {shell, arg0, command, NULL};

    int pid;
    int in_pipe[2];
    int out_pipe[2];

    Stream self = stream_new();
    if (! self) return NULL;

    self->type = 'x';

    signal(SIGPIPE, SIG_IGN);

    pipe(in_pipe);
    pipe(out_pipe);

    pid = fork();


    switch(pid){
        case -1: // ERROR
            return NULL;

        case 0: // CHILD
            close(0); //close childs stdin
            dup(in_pipe[0]); // duplicate read end of in pipe
            close(in_pipe[0]);
            close(in_pipe[1]);

            close(1); //close stdout
            dup(out_pipe[1]);
            close(out_pipe[0]);
            close(out_pipe[1]);

            //execlp(filename, filename,NULL);
            execvp(args[0],args);

            exit(1); //an error occured

        default: // PARENT
            close(in_pipe[0]); // close read end of in_pipe
			close(out_pipe[1]); //close write end of out_pipe
            self->output_descriptor = in_pipe[1];
            self->input_descriptor = out_pipe[0];
            self->pid = pid;
    }

    return self;
}

#endif



void stream_close(Stream self){

    switch (self->type){

        case 'f': //FILE type
            if (self->input_descriptor != -1) close(self->input_descriptor);
            if (self->output_descriptor != -1) close(self->output_descriptor);
            break;

        case 's': // SOCKET
#ifdef __WIN32__
            if (self->input_descriptor != -1) closesocket(self->input_descriptor);
#else
            if (self->input_descriptor != -1){
                shutdown(self->input_descriptor, SHUT_RDWR);
                close(self->input_descriptor);
            }
#endif
            break;

        case 'x':  // EXEC
            close(self->output_descriptor);
            close(self->input_descriptor);
#ifdef __WIN32__
#else
            waitpid(self->pid, NULL, 0);
#endif
            break;

    }
    // MEMORY
    if (self->buffer)free(self->buffer);

    free(self);
}











