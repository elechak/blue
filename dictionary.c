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
#include "dictionary.h"

/* NODES */

static DictionaryNode node_new(string_t key, Link value){
    DictionaryNode self = malloc( sizeof(*self) );
    self->left     = NULL;
    self->right    = NULL;
    self->key      = string_dup(key);
    self->value    = link_dup(value);
    return self;
}

Dictionary dictionary_new(){
    Dictionary self = malloc( sizeof(*self));
    self->size = 0;
    self->root = NULL;
    return self;
}


static int dictionary_splay(Dictionary self, string_t key){
    struct DictionaryNode store;
    int comp;

    if (! self->root) return 1;

    if ( (comp = string_compare(key, self->root->key) ) == 0 ) return 0;
        
    store.left       = NULL;
    store.right      = NULL;
    DictionaryNode left        = &store;
    DictionaryNode right       = &store;
    DictionaryNode temp        = NULL;

    do{

        if (comp < 0){
            if (! self->root->left) break;

            // Right rotate
            if ( (comp = string_compare(key, self->root->left->key)) < 0 ){
                temp = self->root->left;
                self->root->left = temp->right;
                temp->right = self->root;
                self->root = temp;
                if (! self->root->left) break;
            }

            right->left = self->root;
            right = self->root;
            self->root = self->root->left;

        }else if (comp > 0){
            if (! self->root->right) break;

            // Left rotate
            if ( (comp = string_compare(key, self->root->right->key)) > 0 ){
                temp = self->root->right;
                self->root->right = temp->left;
                temp->left = self->root;
                self->root = temp;
                if (! self->root->right) break;
            }

            left->right = self->root;
            left = self->root;
            self->root = self->root->right;
        }

    }while( (comp = string_compare(key, self->root->key)) );

    left->right = self->root->left;
    right->left = self->root->right;
    self->root->left = store.right;
    self->root->right = store.left;
    return comp;
}


Link dictionary_get(Dictionary self, string_t key){
    if (! self->root) return NULL;
    if ( dictionary_splay(self, key)==0) return link_dup(self->root->value);
    return NULL;
}


// returns a the old value if the key was already used
Link dictionary_insert(Dictionary self, string_t key, Link value){
    DictionaryNode temp = NULL;
        
    int comp;

    // Empty tree so just make the new node the root
    if (! self->root) {
        self->root = node_new(key,value);
        self->size++;
        return NULL;
    }

    comp = dictionary_splay(self, key);

    if (comp < 0){
        temp=self->root;
        self->root = node_new(key,value);
        self->root->left = temp->left;
        temp->left = NULL;
        self->root->right = temp;
        self->size++;
    }else if (comp > 0){
        temp=self->root;
        self->root = node_new(key,value);
        self->root->right = temp->right;
        temp->right = NULL;
        self->root->left = temp;
        self->size++;
    }else{
        Link tempvalue = self->root->value;
        self->root->value = link_dup(value);
        return tempvalue;
    }

    return NULL;
}


Link dictionary_delete(Dictionary self, string_t key){
    DictionaryNode temp  = NULL;
    Link value = NULL;
    
    if ( ( ! self ) || (! self->root) )  return NULL;

    if ( ! key ) key = self->root->key;
    
    if ( dictionary_splay(self, key) )  return NULL;

    // found exact match
    temp = self->root;
    value = self->root->value;

    if (temp->left == NULL){
        if (temp->right){
            self->root = temp->right;
        }else{
            self->root = NULL;
        }
    }else if (temp->right == NULL){
        if (temp->left){
            self->root = temp->left;
        }else{
            self->root = NULL;
        }
    }else{
        self->root = temp->left;
        dictionary_splay(self, key);
        self->root->right = temp->right;
    }
    self->size--;

    free(temp->key);
    free(temp);
    
    return value;
}


void dictionary_clear(Dictionary self){
    while( self->root){
        link_free(dictionary_delete(self, self->root->key));
    }
    self->size = 0;
}


void dictionary_free(Dictionary self){    
    dictionary_clear(self);
    free(self);
}

static void _each( Dictionary self, DictionaryNode n, void(*func)(string_t, Link, void *), void * arg ){
        func(n->key, n->value, arg);
        if (n->left)  _each( self, n->left , func , arg );
        if (n->right) _each( self, n->right , func , arg);
}

void dictionary_each( Dictionary self, void(*func)(string_t, Link, void *), void * arg ){
    if ((self) && (self->root)) _each(self, self->root, func, arg);
}


static void _getKeys(string_t key, Link value, void * keys){
    stack_push((LinkStack)keys,   create_string_str(string_dup(key))   );
}

Link dictionary_getKeys(Dictionary self){
    if (! self) return array_new(0);
    stack_init( keys , LinkStack, self->size);
    dictionary_each(self, _getKeys, keys);
    return array_new_attach(keys);
}

static void _getValues(string_t key, Link value, void * values){
    stack_push((LinkStack)values, link_dup(value));
}

Link dictionary_getValues(Dictionary self){
    if (! self) return array_new(0);
    stack_init( values , LinkStack, self->size);
    dictionary_each(self, _getValues, values);
    return array_new_attach(values);
}


























