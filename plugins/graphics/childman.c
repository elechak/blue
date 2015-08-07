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


#include "graphics.h"

#define BEVT_DELIM " -+\t\n\r"

static ListNode getChildNode(Link self,Link what){

    ChildMan cm = self->value.vptr;

    List children = cm->children;
    ListNode child_node = NULL;
    size_t count;
    size_t index;

    if (what->type == Global->number_type){
    /* NUMBER INDEX */
        index = (int)object_asNumber(what);
        if (index >= list_getLength(children)) return NULL;
        goto have_index;
    }else if (    object_getString( what ) ){
    /* STRING INDEX */

        for( count=0 ; count < list_getLength(children) ; count++){
            child_node = *((ListNode *)list_get(children, count));

            if (child_node && ( string_compare(child_node->name, object_getString( what ) )==0)){
                index = count;
                goto have_index;
            }
        }
    }

    return NULL;

    have_index:

    return *((ListNode *)list_get(children, index));
}


static Link getChild(Link self,Link what){
    
    Link * args = array_getArray(what);
    size_t argn  = array_getLength(what);    
    
    if (argn != 1) return NULL;
    if ((   ! object_getString(args[0]) ) && (args[0]->type !=Global->number_type)){
        return NULL;        
    }
    
    ListNode child_node = getChildNode(self,args[0]);

    if (child_node){
        return link_dup(child_node->link);
    }else{
        return NULL;
    }
}

static Link delChild(Link self,Link what){
    
    Link * args = array_getArray(what);
    size_t argn  = array_getLength(what);    
    
    if (argn != 1) return NULL;
    
    ChildMan cm = self->value.vptr;
    List children = cm->children;

    ListNode child_node = NULL;
    Link child = NULL;
    size_t count;
    size_t index;

    if (args[0]->type == Global->number_type){
    /* NUMBER INDEX */
        index = (int)object_asNumber(args[0]);
        goto have_index;
    }else if (object_getString( args[0] ) ){
    /* STRING INDEX */
        for( count=0 ; count < list_getLength(children) ; count++){
            child_node =*((ListNode *)list_get(children, count));
            if (child_node && ( string_compare(child_node->name, object_getString( args[0] ) )==0)){
                index = count;
                goto have_index;
            }
        }
        return NULL;
    }else{
        return NULL;
    }

    return NULL;

    have_index:

    /* ensure that the index is within the list's bounds */
    if (list_getLength(children) >= index) return NULL;

        /* get then delete the child node from the list */
        if (! child_node) child_node  = *((ListNode *)list_get(children, index));
        list_delete(children, index);

        /* save the link */
        child = child_node->link;

        /* free the name string and the node */
        free(child_node->name);
        free(child_node);

        /* return the link */
        return child;
}


static Link addChild(Link self,Link child,Link what){
    
    Link * args = array_getArray(what);
    size_t argn  = array_getLength(what);    
    
    if (argn != 1) return NULL;    
    
    ChildMan cm = self->value.vptr;
    List children = cm->children;
    ListNode child_node = NULL;

    if (object_getString( args[0] ) ){
    /* STRING INDEX */
        child_node = getChildNode(self, args[0]);

        /* child already exists, so free the old one and replace with the new one */
        if (child_node){
            link_free(child_node->link);
            child_node->link = link_dup(child);
        }else{
        /* child does not exist so append it */
            child_node = malloc(sizeof(*child_node));
            child_node->link = link_dup(child);
            child_node->name = string_dup(object_getString(args[0]) );
            list_append(children, &child_node);
        }
        return child;
    }

    return exception("BadIndexType", NULL, NULL);
}


void drawChildren(ChildMan parent){

    if ( (! parent->children) || (! parent->children->length)) return;

    size_t length = parent->children->length;
    ListNode *nodes = (ListNode *)parent->children->data;
    ListNode node = NULL;

    int c;
    for(c=0; c<length; c++){
        node = nodes[c];
        if (node->link->type->draw){
            glPushMatrix();
            object_draw(node->link);
            glPopMatrix();
        }
    }
}

void freeChildren(ChildMan parent){
    if (! parent->children) return;

    size_t length = parent->children->length;
    ListNode *nodes = (ListNode *)parent->children->data;
    ListNode node = NULL;
    int c;
    for(c=0; c<length; c++){
        node = nodes[c];
        free(node->name);
        link_free(node->link);
    }
    list_free(parent->children);
}


void child_manager(NativeType t){
    t->getChild    = getChild;
    t->addChild   = addChild;
    t->delChild   = delChild;
}


