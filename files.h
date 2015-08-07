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




#ifndef _FILES
#define _FILES

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

char * file_load(const char * filename);
char * file_cd_up(const char * path);
char * file_cd_into(char * part1 , char * part2);
int file_copy(char * source, char * dest);
char * file_name(char * filename);
int file_isexecutable(char * filename, char * env);
int file_exists(char * filename);
int file_exists2(char * part1, char * part2);
off_t file_size(char * filename);
char * file_pathto(char * filename, char * env);
#endif

