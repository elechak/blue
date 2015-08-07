
#include "graphics.h"

ItemManager im_new(){
    ItemManager self = malloc ( sizeof( *self));
    self->capacity = 0;
    self->size = 0;
    self->items = NULL;
    return self;
}

void im_free( ItemManager self){
    int c;
    for ( c=0; c<self->size; c++){
        link_free( self->items[c]);
    }
    free(self);
}


void im_draw(ItemManager self){
    int c;
    for ( c=0; c<self->size; c++){
        glPushMatrix();
        object_draw(self->items[c]);        
        glPopMatrix();
    }
}


void im_append( ItemManager self, Link item){
    if ( self->size == self->capacity) {
        self->items = realloc( self->items, (self->capacity += 8) * ( sizeof(Link) ) );
    }    
    self->items[self->size++] = link_dup(item);
};