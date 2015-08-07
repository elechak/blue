/*
The blue programming language ("blue")
Copyright (C) 2007-2008  Erik R Lechak

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

#include "../graphics.h"
#include <png.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#define HEADERSIZE 4


static NATIVECALL(png_new){
    Link * args = array_getArray(Arg);

    unsigned char pngheader[HEADERSIZE];
    png_structp png;
    png_infop info;
    png_bytep * row_pointers;
    png_byte* row ;

    FILE *fp;
    int  y, width, height;

    /* open the file args[0] is the filename */
    string_t filename_string = object_getString(args[0]);
    fp = fopen(filename_string->data , "rb");
    if (! fp){
        return exception("CouldNotOpenFile", NULL, NULL);
    }

    /* read the png header and ensure it is a png */
	fread(pngheader, 1, HEADERSIZE, fp);
	if (png_sig_cmp(pngheader, 0, HEADERSIZE)){
        return exception("NotPNG", NULL, NULL);
    }

	png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_set_sig_bytes(png, HEADERSIZE);
	info = png_create_info_struct(png);

	png_init_io(png, fp);

	png_read_info(png, info);

    Link obj = object_create(image_type);

    Image self = obj->value.vptr;

    self->width = info->width;

    self->height = info->height;

	width = info->width;

	height = info->height;

    //info->bit_depth;
    //info->channels;
	//info->color_type;
    //printf(" bit depth:%i    channels:%i    color type:%i   rowbytes:%i\n", info->bit_depth, info->channels, info->color_type , info->rowbytes);


    /* read png information */
	png_read_update_info(png, info);

    /* allocate memory for rows */
	row_pointers = malloc(sizeof(png_bytep) * height);
	for (y=0; y<height; y++) row_pointers[y] = malloc(info->rowbytes);

    /* read image into allocated memory */
	png_read_image(png, row_pointers);

    fclose(fp);

    png_byte * pdata;

    self->format = info->channels;
    self->data = pdata = malloc(self->width * self->height * self->format);
    for (y=0; y<height; y++) {
        row = row_pointers[height-y-1];
        memcpy(pdata,row, self->width * self->format);
        pdata+=self->width * self->format;
        free(row);
    }

    free(row_pointers);
    png_destroy_read_struct(&png, &info, NULL);

    return obj;
}


void init(INITFUNC_ARGS){
    addCFunc(Module, "new", png_new);
}

