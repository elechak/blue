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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../graphics.h"


static NATIVECALL(shape_sphere){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    Link link = object_create(shape_type);
    Shape self = link->value.vptr;

    int n = 20;

    if (argn == 1){
        n = (int)object_asNumber(args[0]);
    }

    float radius = 1.0;
    float center[] = {0.0, 0.0, 0.0};

    float twopi = M_PI * 2;

    List normals   = self->normals;
    List vertices  = self->vtx;
    List texcoords = self->texcoords;

    struct Xyz  vertex;
    struct Xyz  normal;
    struct Xy   texcoord;

    float j, i;
    float theta1, theta2, theta3;

    for ( j = -n/4.0 ; j < n/4.0 ; j++){
        theta1 = j * twopi/n;
        theta2 = (j+1) * twopi/n;
        for( i=0 ; i < n+1 ; i++){

            theta3 = i * twopi/n;

            if (i == n)   theta3 = 0;

            normal.x = cos(theta2)*cos(theta3);
            normal.y = sin(theta2);
            normal.z = cos(theta2)*sin(theta3);

            vertex.x = center[0] + radius*normal.x;
            vertex.y = center[1] + radius*normal.y;
            vertex.z = center[2] + radius*normal.z;

            texcoord.x = 1.0 - (i/(float)n);
            texcoord.y = 0.5 + (2*(j+1)/(float)n);

            list_append(normals,   &normal);
            list_append(vertices,  &vertex);
            list_append(texcoords, &texcoord);

            normal.x = cos(theta1)*cos(theta3);
            normal.y = sin(theta1);
            normal.z = cos(theta1)*sin(theta3);

            vertex.x = center[0] + radius*normal.x;
            vertex.y = center[1] + radius*normal.y;
            vertex.z = center[2] + radius*normal.z;

            texcoord.x = 1.0 - (i/(float)n);
            texcoord.y = 0.5 + (2*j/(float)n);

            list_append(normals,&normal);
            list_append(vertices, &vertex);
            list_append(texcoords, &texcoord);
        }
    }

    return link;
}




//~ static NATIVECALL(shape_partialSphere){
    //~ Link * args = array_getArray(Arg);
    //~ //size_t argn  = array_getLength(Arg);

    //~ Link link = object_create(shape_type);
    //~ Shape self = link->value.vptr;

    //~ int n = 20;
    //~ int slices ;
    //~ float radius = 1.0;
    //~ float center[] = {0.0, 0.0, 0.0};

    //~ float twopi = M_PI * 2;

    //~ List normals   = self->normals;
    //~ List vertices  = self->vtx;
    //~ List texcoords = self->texcoords;

    //~ struct Xyz  vertex;
    //~ struct Xyz  normal;
    //~ struct Xy   texcoord;

    //~ float j, i;
    //~ float theta1, theta2, theta3;

    //~ int sections = 0;
    
    //~ //int ncount;
    
    //~ for ( j = -n/4.0 ; j < n/4.0 ; j++){
        
    //~ printf("j = %g\n", j);
        
    //~ //for ( ncount = -n ; ncount < n ; ncount++){
        
        //~ //j = ncount /4.0;
        
        //~ theta1 = j * twopi/n;
        //~ theta2 = (j+1) * twopi/n;
        //~ //for( i=0 ; i < n+1 ; i++){
        //~ for( i=0 ; i < slices ; i++){

            //~ theta3 = i * twopi/n;

            //~ if (i == n)   theta3 = 0;

            //~ normal.x = cos(theta2)*cos(theta3);
            //~ normal.y = sin(theta2);
            //~ normal.z = cos(theta2)*sin(theta3);

            //~ vertex.x = center[0] + radius*normal.x;
            //~ vertex.y = center[1] + radius*normal.y;
            //~ vertex.z = center[2] + radius*normal.z;
            //~ printf("V1\n");

            //~ texcoord.x = (i/(float)n);
            //~ texcoord.y = 0.5 + (2*(j+1)/(float)n);

            //~ list_append(normals,   &normal);
            //~ list_append(vertices,  &vertex);
            //~ list_append(texcoords, &texcoord);
            //~ //break;
            
            //~ //if (ncount == -n) break;
            
            //~ normal.x = cos(theta1)*cos(theta3);
            //~ normal.y = sin(theta1);
            //~ normal.z = cos(theta1)*sin(theta3);

            //~ vertex.x = center[0] + radius*normal.x;
            //~ vertex.y = center[1] + radius*normal.y;
            //~ vertex.z = center[2] + radius*normal.z;
            //~ printf("V2\n");

            //~ texcoord.x = (i/(float)n);
            //~ texcoord.y = 0.5 + (2*j/(float)n);

            //~ list_append(normals,&normal);
            //~ list_append(vertices, &vertex);
            //~ list_append(texcoords, &texcoord);
        //~ }
        
        //~ sections++;
        //~ if (sections == 1) break;
    //~ }

    //~ return link;
//~ }





void init(INITFUNC_ARGS){
    addCFunc(Module, "sphere", shape_sphere);    
    //addCFunc(Module, "partial_sphere", shape_partialSphere);    
}





