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


#ifndef _SPLAYTREE
#define _SPLAYTREE

#include <stdlib.h>

#include "comparison.h"

struct SplayNode{
    void *             data;
    struct SplayNode *   right;
    struct SplayNode *   left;
};
typedef struct SplayNode * SplayNode;

struct SplayTree{
    size_t size;
    SplayNode root;
    Comparator cmp;
    Destroyer destroyer;
};
typedef struct SplayTree * SplayTree;

#define splaytree_getSize(SPLAYTREE) (SPLAYTREE->size)
SplayTree splaytree_new(Comparator cmp);
void splaytree_setDestroyer(SplayTree tree , Destroyer d);
void splaytree_free(SplayTree tree);
void splaytree_totalFree(SplayTree tree);
void * splaytree_get(SplayTree tree,  void *data);
void * splaytree_insert(SplayTree tree, void *data);
void * splaytree_delete(SplayTree tree, void *key);
void splaytree_each( SplayTree tree, void(*func)(void*) );
void splaytree_each2( SplayTree tree, void(*func)(void*, void*) , void * arg);
void splaytree_clear(SplayTree tree);

#endif
