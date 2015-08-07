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

#ifndef _DICTIONARY
#define _DICTIONARY

#include "global.h"

#include "threading.h"
#include "bstring.h"


struct DictionaryNode{
    string_t                  key;
    Link                      value;
    struct DictionaryNode *   right;
    struct DictionaryNode *   left;
};
typedef struct DictionaryNode * DictionaryNode;

struct Dictionary{
    size_t size;
    DictionaryNode root;
};
typedef struct Dictionary * Dictionary;

EXPORT Dictionary dictionary_new();
EXPORT Link dictionary_get(Dictionary self, string_t key);
EXPORT Link dictionary_insert(Dictionary self, string_t key, Link value);
EXPORT Link dictionary_delete(Dictionary self, string_t key);

EXPORT void dictionary_clear(Dictionary self);
EXPORT void dictionary_free(Dictionary self);  

EXPORT Link dictionary_getKeys(Dictionary self);
EXPORT Link dictionary_getValues(Dictionary self);

EXPORT void dictionary_each( Dictionary self, void(*func)(string_t, Link, void *), void * arg );

#endif
