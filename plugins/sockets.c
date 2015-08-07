/*
The blue programming language ("blue")
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
#include "../global.h"
#include <stdarg.h>
#include <signal.h>
#include <unistd.h> // close
#include <fcntl.h> // O_RDWR
#include "../bstring.h"

// ----------------------- WINDOWS ---------------------------
#ifdef __WIN32__
#include <windows.h>
#include <winsock.h>
#include <io.h>
#else
// ----------------------- LINUX ---------------------------------
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h> // gethostbyname
#endif


#ifdef __WIN32__
    WSADATA wsaData;
    int wsaStarted;
#endif

EXPORT void init(INITFUNC_ARGS);


static NativeType socket_type = NULL;


static struct sockaddr_in resolvebyname(char * host){
    struct sockaddr_in  address;
    struct hostent * server;
	char hostname[64];
	
    int status = strcspn(host,":");
    strncpy(hostname, host,status);
    hostname[status] = '\0';
	
	server = gethostbyname(hostname);
	
    int port =  atoi( host+1+status );	
	address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    memcpy(&address.sin_addr.s_addr,server->h_addr,server->h_length );
	return address;
}

string_t addrtostring(struct sockaddr address){
		struct sockaddr_in * in;
		int port = 0;
		char buffer[64];
		buffer[0] = 0;
	
		switch( address.sa_family){
			case AF_INET:
				in = (struct sockaddr_in *)& address;
				inet_ntop(AF_INET, &(in->sin_addr.s_addr), buffer,63);
				port = ntohs(in->sin_port);
				break;
		}
		return string_new_formatted("%s:%i",buffer,port);
}


static void destroy(Link link){
	close( link->value.i);
	object_destroy(link);
}

static NATIVECALL(n_tcp){
	Link link = object_create( socket_type);
	link->value.i = socket(PF_INET,SOCK_STREAM,0);  //TCP	
    return link;
}

static NATIVECALL(n_udp){
	Link link = object_create( socket_type);
	link->value.i = socket(PF_INET,SOCK_DGRAM,0);  //UDP
    return link;
}

static NATIVECALL(n_bind){
	struct sockaddr_in address;
	
    Link * args = array_getArray(Arg);
	int port = (int)(args[0]->value.number);
	
	bzero(&address,sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr=htonl(INADDR_ANY);
	address.sin_port=htons(port);	
	
	bind(This->value.i,(struct sockaddr *)&address,sizeof(address));
	
    return link_dup(This);
}



static NATIVECALL(n_connect){
	struct sockaddr_in address;
    Link * args = array_getArray(Arg);
    string_t host_address = object_getString( args[0] );
	address = resolvebyname(host_address->data);
	connect( This->value.i , (struct sockaddr *)&address,sizeof(address));
    return link_dup(This);
}


static NATIVECALL(n_listen){
    Link * args = array_getArray(Arg);
    int backlog = (int)(args[0]->value.number);
	listen( This->value.i , backlog);
    return link_dup(This);
}


static NATIVECALL(n_accept){
	struct sockaddr_in address;
	socklen_t len = sizeof(address);
	int i = accept( This->value.i, (struct sockaddr *)&address, &len);
	Link link = object_create( socket_type);
	link->value.i = i;
    return link;
}

static NATIVECALL(n_send){
	Link * args = array_getArray(Arg);
	string_t payload = object_getString(args[0]);
	int c = send( This->value.i, payload->data,payload->length,0);
	return create_numberi(c);
}

static NATIVECALL(n_recv){
	struct sockaddr address;
	socklen_t len = sizeof(address);
	char buffer[1000];
	int c = recvfrom( This->value.i, buffer,1000,0,&address, &len);
	
	Link link = create_string_str(string_new_ls(c,buffer));
	addAttr(link, create_string_str(addrtostring(address)) , create_string("address"));
	return link;
}

static NATIVECALL(n_sendto){
	struct sockaddr_in address;
	Link * args = array_getArray(Arg);
	string_t host_address = object_getString( args[0] );
	address = resolvebyname(host_address->data);
	string_t payload = object_getString(args[1]);
	int c = sendto( This->value.i, payload->data,payload->length,0,(struct sockaddr *)&address,sizeof(address));
	return create_numberi(c);
}



void init(INITFUNC_ARGS){
    
	socket_type = newNative();
        socket_type->destroy  = destroy;
	
		addNativeCall(socket_type, "connect", n_connect);
		addNativeCall(socket_type, "accept", n_accept);
		addNativeCall(socket_type, "bind", n_bind);
		addNativeCall(socket_type, "listen", n_listen);
		addNativeCall(socket_type, "send", n_send);
		addNativeCall(socket_type, "recv", n_recv);
		addNativeCall(socket_type, "sendto", n_sendto);
	
	
	#ifdef __WIN32__
	// Initialize winsock sockets
	if ( ! wsaStarted){
		if ( (WSAStartup(0x0101, &wsaData) != NO_ERROR) ){
			printf("WSAStartup failed\n");
		}else{
			wsaStarted =1;
		}
	}
	#endif
		
	
    /* MODULE FUNCTIONS */
        addCFunc(Module, "tcp",   n_tcp);
        addCFunc(Module, "udp",   n_udp);
        //addCFunc(Module, "listen",   udp_listen);
}


