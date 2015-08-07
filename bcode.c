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

#include "global.h"
#include "files.h"

static NativeType t;
static Dictionary  modules;
static Mutex modules_mutex;
static string_t module_dir;



static void create(Link self){
    Module module  = self->value.module = malloc( sizeof(*module) );

    module->name             = NULL;  
    module->bytecode         = NULL;   
    module->global_vars      = NULL;   
    module->bytecode_size    = 0;
}


static void destroy(Link self){
    int i;
    Module module = self->value.module;
    if (module){
        if (module->name) free(module->name);
        
        if (module->global_vars){
            for( i=0; i < module->global_vars->size ; i++){
              link_free( module->global_vars->links[i]);  
            };
            free(module->global_vars);
        }
        
        if (module->bytecode) free(module->bytecode);    
        
        free(self->value.module);
    }
    object_destroy(self);    
}

static Link call(Link self,  Link This, Link Arg){
    Link cbo = object_create(Global->function_type);
    CodeBlock cb = cbo->value.codeblock;

    cb->parent   = self;
    cb->bytecode = self->value.module->bytecode;

    Link ret = object_call(cbo, self,Arg);

    link_free(cbo);
    return ret;
}

/* Compile a string into a module */
Link create_module_string(const string_t source){
    
    /* compiler the source to assembly code */
        char * acode = compile_cstr(source->data);
    
    /* See if this is an error */
        if ( memcmp( acode, "ERR:",4) ==0 ){            
            Link e =  exception(acode+4 , NULL, NULL);
            free( acode );
            return e;
        }    
    
    /* Assemble the assembly code to bytecode*/
        Bytes bytes = assemble(acode);    
    
    /* cleanup */
        free( acode);
    
    /* Create the module object */
        Link module = object_create(Global->module_type);

    /* Attach the compiled bytecode to the module */
        module->value.module->bytecode_size = bytes->write_offset;
        module->value.module->bytecode = bytes_detach(bytes);     

    return module;    
}

/* Compile a file into a module */
Link create_module_filename(const char * filename){
    Link module = NULL;

    /* Load file into 'source' */    
    string_t source = string_load( filename );
    
    /* check to make sure it is not compiled */
    if  ((source->length > 3) &&  
        (source->data[0] == INTRO)&&
        ( ((unsigned char)source->data[1]) == 234)&&
        (source->data[2] == 7)
    ){
    /* IT IS COMPILED */
        module = object_create(Global->module_type);
        module->value.module->bytecode = malloc( sizeof(bytecode_t) * source->length);
        memcpy( module->value.module->bytecode , source->data, source->length);
        module->value.module->bytecode_size = source->length;
    }else{
    /* NOT COMPILED */
        module = create_module_string(source);
    }
    
    free(source);
    return module;
}

void module_save( Link module, char * filename ){
    Bytes bytes = bytes_attach( module->value.module->bytecode, module->value.module->bytecode_size);
    bytes_save( bytes,filename);
    bytes_detach( bytes );
}


static inline Link module_exist( string_t key){

    mutex_lock(modules_mutex);

    Link module_link = dictionary_get(modules, key);
    if ( module_link ){
        mutex_unlock(modules_mutex);    
        free(key);
        return module_link;
    }

    mutex_unlock(modules_mutex);     
    return NULL;
}



Link module_new(char * filename, Link args){

    string_t key= string_new(filename);
    
    /* check to see if this module was loaded before */
    Link module_link = module_exist(key);
        if (module_link) return module_link;

    /* if you got here the requested module was not loaded before */
    
    /* find the modules location, either in the current directory or blue/module directory */
    char * real_filename=NULL;

    if ( file_exists(filename) ){
        real_filename = filename;
    }else if ( file_exists2( module_dir->data, filename) ){
        real_filename =  file_cd_into(module_dir->data, filename );
    }else{
        free(key);
        return NULL;
    }
       
    module_link = create_module_filename( real_filename );
        if ( is_critical(module_link) ) return module_link;
        if (! module_link) return NULL;
    
    /* Create module object */
    Module module = module_link->value.module;
    module->name = key;

    /* add module to the modules lookup table */
        dictionary_insert(modules, key, link_dup(module_link));

    /* call the module to initialize it */
    Link ret = call(module_link, link_dup(module_link),args);

    if (ret){
        link_free(module_link);
        return ret;
    } 
    return module_link;    
}


Link library_new(char * filename){
    
    string_t key = string_new(filename);
    
    /* check to see if this module was loaded before */
    Link module_link = module_exist(key);
        if (module_link) return module_link;    
    
    /* if you got here the requested module was not loaded before */
    
    /* Create module object */
        module_link = object_create(Global->module_type);
        Module module = module_link->value.module;    
        module->name = key;
    
    /* load the library into this module */
        char * temp = file_cd_into(Global->blue_lib_location->data, filename );
        int load_error;
        
        //printf("AA %s \n", temp);
    
        if (temp){
            load_error = loadNative(temp, module_link);
            free(temp);
        }else{
            load_error = loadNative(filename, module_link);
        }

        
        
        if (load_error){
            link_free(module_link);
            return exception("LibraryNotFound", NULL, NULL);
        }
        
    /* add module to the modules lookup table */
        dictionary_insert(modules, key, link_dup(module_link));

    return module_link;    
}


static NATIVECALL(module_create){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    
    size_t c;
    size_t size  = 0;

    size = argn-1;
    
    if (! argn ) goto error;
    
    /* create new array for arguments */
    Link newargs_link = array_new(argn-1);
    Link * links = newargs_link->value.array->links;
    
    //Array newargs = newargs_link->value.array;
    for(c=0; c< size; c++){
        links[c] = link_dup(args[c+1]);
    }
    
    /* create the module */
    Link mod = NULL;

    string_t name = object_getString( args[0] );
    if ( name ) mod = module_new( name->data , newargs_link );    
    
    link_free(newargs_link);
    
    if (! mod ) goto error;
    return mod;
    
    error:
    return exception("ModuleNotFound", NULL, NULL);        
};


static string_t library_postfix= NULL;

static NATIVECALL(library_create){
    Link * args = array_getArray(Arg);
    Link lib = NULL;
    string_t libname = object_getString( args[0] );
    
    if ( ! string_endsWith(libname, library_postfix)){
        libname = string_new_merge(libname, library_postfix, 1,1);
        lib = library_new( libname->data );
        free(libname);
    }else{
        lib = library_new( libname->data );
    }
    
    return lib;
};


Link module_import(Link filename, Link args){
    /* create the module */
    Link mod = NULL;

    string_t name = object_getString( filename );
    
    if ( ! string_endsWith(name, library_postfix)) mod = module_new( name->data , args );    
    
    if (mod) return mod;
    
    if ( ! string_endsWith(name, library_postfix)){
        name = string_new_merge(name, library_postfix, 1,1);
        mod = library_new( name->data );
        free(name);
    }else{
        mod = library_new( name->data );
    }    
    
    if (mod) return mod;
    
    return exception("ImportFailed", object_getString(filename), NULL);
}



static NATIVECALL(import_plugin){
    //Link * args = array_getArray(Arg);
    
    Link mod = NULL;
    
    /* Try loading a module */
        mod = module_create(This, Arg);        
        if ( ! is_critical(mod) ) return mod;
        link_free(mod);
    
    /* Try loading a library */
        mod = library_create(This, Arg);
        if ( ! is_critical(mod) ) return mod;
        link_free(mod);
    
    /* Couldn't load requested plugin */
        Link * args = array_getArray(Arg);
        return exception("ImportFailed", object_getString(args[0]), NULL);
};



static NATIVECALL(id){
    Link * args = array_getArray(Arg);
    return create_numberul((unsigned long) (args[0]) );
};


static NATIVECALL(sys_print){    
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    size_t count;
    string_t s = NULL;

    for ( count = 0; count <argn; count++){
        s = object_asString(args[count]);
        fwrite(s->data, 1,s->length,stdout);
        free(s);
    }
    fflush(stdout);
    return object_create(Global->null_type);
};


static NATIVECALL(system_sleep){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);
    number_t num;

    if (argn){
        num =  args[0]->value.number;
    }else {
        num = 1.0;
    }

#ifdef __WIN32__
    Sleep((unsigned int) (num*1000));
#else
    usleep((unsigned int) (num*1000000));
#endif
    return object_create(Global->null_type);
};

static NATIVECALL(refcount){
    Link * args = array_getArray(Arg);
    return create_numberi( args[0]->refcount-1);
}

static NATIVECALL(system_exec){
    Link * args = array_getArray(Arg);
    string_t s = args[0]->value.string;
    return create_numberi( system(s->data)  );
};

static NATIVECALL(system_exit){
    exit(0);
    return object_create(Global->null_type);
};


static NATIVECALL(getAttrs){
    Link * args = array_getArray(Arg);
    size_t argn  = array_getLength(Arg);

    Link ret = NULL;
    
    switch (argn){
        case 1:
            return getAttrKeys(args[0]);
        case 2:
            ret = getAttr(args[0], args[1]) ;
            if (! ret) ret = exception("AttrNotFound", NULL, NULL);
            return ret ;
        default:
            return exception("NumberOfArgumentsError",NULL,NULL);        
    }
}





NativeType module_init(){
    
    library_postfix = string_new(".dll");
    
    module_dir = string_new_merge(Global->blue_location, string_new("/module/") ,1,0);
    
    t = newNative();
        t->create         = create;
        t->destroy        = destroy;
    
    modules = dictionary_new();
    modules_mutex = mutex_new();
    
    Global->SYS_MODULE = object_create( t );
       
    addCFunc(Global->SYS_MODULE, "import", import_plugin);
    addCFunc(Global->SYS_MODULE, "module", module_create);
    addCFunc(Global->SYS_MODULE, "mod", module_create);
    addCFunc(Global->SYS_MODULE, "library", library_create);
    addCFunc(Global->SYS_MODULE, "lib", library_create);
    
    addCFunc(Global->SYS_MODULE,"refcount", refcount);
    addCFunc(Global->SYS_MODULE,"id", id);
    addCFunc(Global->SYS_MODULE,"print", sys_print);

    addCFunc(Global->SYS_MODULE,"exec", system_exec);
    addCFunc(Global->SYS_MODULE,"exit", system_exit);
    addCFunc(Global->SYS_MODULE,"sleep", system_sleep);
    
    addCFunc(Global->SYS_MODULE,"attribs", getAttrs);    
    
    return t;
}


