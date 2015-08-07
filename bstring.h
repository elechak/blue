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
#ifndef _STRING_
#define _STRING_
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>


// Strings are immutable (well, unless you mute them:)
// so all of the functions below generate a new string they don't alter the current string

struct string_header{
    size_t length;
    char data[];
};

typedef struct string_header  *string_t;

// STRING_T CONSTRUCTORS
string_t string_new(const char * cstring);                                  // string from null terminated c string
string_t string_new_ls(size_t length, const char * cstring);                // string from length and bytes
string_t string_new_quoted(const char * cstring, char ** pcstring);         // string from quoted string, pcstring points to end of parse
string_t string_new_formatted(const char * format, ...);                    // string from formatted c string
string_t string_new_add(string_t a, string_t b);                            // string from two strings concatinated, original strings not destroyed
string_t string_new_merge(string_t a, string_t b, int keep_a, int keep_b);  // string from two strings, keep 1=don't destroy, 0=destroy
string_t string_substr(const string_t s, size_t start, size_t end);         // string from substring of original
string_t string_dup(const string_t s);                                      // string duplicate of original

string_t string_ltrim( string_t self, string_t trash);
string_t string_rtrim( string_t self, string_t trash);
string_t string_trim(string_t self, string_t trash);

string_t string_replace( string_t self, string_t pattern , string_t b);

void string_fprint(const string_t self ,FILE * stream);
size_t string_sizeof(const string_t self);
size_t string_length(const string_t self);

int string_compare(string_t a, string_t b);
int string_compare_cstring(string_t a, char * cstring);

int string_startsWith(string_t , string_t);
int string_endsWith(string_t , string_t);

size_t string_find(string_t big, string_t little);

string_t string_toUpper( const string_t s);
string_t string_toLower( const string_t s);

// Load and Save to File
size_t string_save( string_t self, char * filename);
string_t string_load( const char * filename);

#endif

