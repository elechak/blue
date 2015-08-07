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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include "files.h"

#include "compiler.h"

static inline void free_array( char ** array, int index ){
    int c;
    for( c=0; c<index; c++){
        if (array[c]) free( array[c]);
    }
}

#define GLOBAL_TOK    11
#define LEXICAL_TOK   12
#define LOCAL_TOK     13
#define ARG_TOK       14
#define AUTO_TOK      15

/* Token */
struct Token{
    int type;
    int id_type;
    int linenum;
    int length;
    char * string;
    double number;
};

typedef struct Token * Token;

/* State - maintains the state of the parser */
struct State{
    int linenum;
    int debug_linenum;
    char *source;
    char *error;
    char *output;
    int label;
    int cond_stack[128];
    int cond_stack_index;
    int jump_stack[128];
    int jump_stack_index;
    int arg_stack[128];
    int arg_stack_index;
    Token function_stack[128];
    int function_stack_index;
    
    char * locals[128];  // storage for local variable names
    int locals_index;    // current position to enter new local var in locals array
    int local_num[128];  // count of how many locals in current function
    int local_num_index; // index of the function level

    char * args[128];  // storage for arg variable names
    int args_index;    // current position to enter new arg var in locals array
    int args_num[128];  // count of how many args in current function
    int args_num_index; // index of the function level

    char * globals[128];
    int globals_index;
    
    char * lexicals[128];
    int lexicals_index;
    int  lexical_num[128];  // number of local lexicals in current function
    int  lexical_num_index;
    int  lexical_num_total; // total number of active lexical variables        
};
typedef struct State * State;


State state_new(){
    State self = malloc( sizeof(*self) );
    
    self->source  = NULL;
    self->error   = NULL;
    self->output  = NULL;
    
    self->label = 1;
    self->linenum = 1;
    self->debug_linenum = 0;
    self->cond_stack_index = 0;
    self->arg_stack_index = 0;
    self->function_stack_index = 0;
    
    self->jump_stack_index  = 0;
    
    self->globals_index     = 0;
    
    self->locals_index      = 0;
    self->local_num_index   = 0;
    self->local_num[0]      = 0;
    
    self->args_index       = 0;
    self->args_num_index   = 0;
    self->args_num[0]      = 0;    
    
    self->lexicals_index    = 0;
    self->lexical_num_index = 0;
    self->lexical_num[0]    = 0;
    self->lexical_num_total = 0;    

    return self;
}

char * state_free(State self){
    free_array( self->locals, self->locals_index);
    free_array( self->args, self->args_index);
    free_array( self->globals, self->globals_index);
    free_array( self->lexicals, self->lexicals_index);
    
    char * output = self->output;
    
    if (self->error){
        free(output);
        output = self->error;
    }
    
    free(self);
    return output;
}

void state_start_function(State self){
    /* keep track of the number of the types of variables in the current function*/
    self->lexical_num[++self->lexical_num_index] = 0;
    self->local_num[++self->local_num_index] = 0;
    self->args_num[++self->args_num_index] = 0;
}


int state_end_lexicals(State self){
    int c;

    /* get the number of lexical variables declared in this function */
    int i = self->lexical_num[self->lexical_num_index];   
    self->lexical_num_index--;
    
    /* reduce the number of total lexicals by the number contained in the current function */
    self->lexical_num_total -= i;
    
    int end = self->lexicals_index;
    c = (self->lexical_num_total) ? (end - i)  : 0;    
    
    for( ; c < end; c++){
        if (self->lexicals[c])free(self->lexicals[c]);
        self->lexicals[c] = NULL;        
    };
    
    if (! self->lexical_num_total){
        c = self->lexicals_index;
        self->lexicals_index = 0;
        return c;
    }
    
    return 0;
}


int state_end_locals(State self){
    int c;

    /* get the number of local variables declared in this function */
    int i = self->local_num[self->local_num_index];    
    
    int end = self->locals_index;
    c = end - i;    
    
    for( ; c < end; c++){
        if (self->locals[c]) free(self->locals[c]);
        self->locals[c] = NULL;
    };
    
    self->local_num_index --;
    self->locals_index -= i;
    
    return i;
}

int state_end_args(State self){
    int c;

    /* get the number of local variables declared in this function */
    int i = self->args_num[self->args_num_index];    
    
    int end = self->args_index;
    c = end - i;    
    
    for( ; c < end; c++){
        if (self->args[c]) free(self->args[c]);
        self->args[c] = NULL;
    };
    
    self->args_num_index --;
    self->args_index -= i;
    
    return i;
}


int state_end_globals(State self){
    int c;

    for( c=0 ; c < self->globals_index ; c++){
        if (self->globals[c]) free(self->globals[c]);
        self->globals[c] = NULL;
    };
    
    return self->globals_index;
}



int state_add_function(State self, Token tok){
    self->function_stack[ self->function_stack_index] = tok;
    self->function_stack_index++;
    return self->label++;
}


/* Handling argument lists : (1,2,3,4) */
void state_add_arguments( State self){
    self->arg_stack[self->arg_stack_index++ ]     = 0;
}

int state_inc_arguments( State self ){
    return self->arg_stack[ self->arg_stack_index - 1 ] +=1  ;
}

int state_pop_arguments( State self ){
    return self->arg_stack[ -- self->arg_stack_index ];
}


/* Handling condition jumps : ? and or */
int state_push_cond( State self){
    int label = self->label++;
    self->cond_stack[ self->cond_stack_index++ ] = label;
    return label;
}

int state_pop_cond( State self){
    return self->cond_stack[ --self->cond_stack_index ];
}

int state_push_jump( State self){
    int label = self->label++;
    self->jump_stack[ self->jump_stack_index++ ] = label;
    return label;
}

int state_pop_jump( State self){
    return self->jump_stack[ --self->jump_stack_index ];
}

int state_get_jump( State self){
    return self->jump_stack[ self->jump_stack_index-1 ];
}



/* TOKENS */
Token token_new(){
    Token self = malloc( sizeof( *self));
    //printf("token new %p\n", self);
    self->type    = 0;
    self->id_type = 0;
    self->linenum = 0;
    self->length  = 0;
    self->string  = NULL;
    self->number  = 0;
    return self;
}

static inline void token_free_string(Token self){
    if (self->string) free( self->string);
    self->string = NULL;
}

void token_free( Token self){
    token_free_string(self);
    //printf("token free  %p\n", self);
    free(self);
}

void token_setString( Token self , char * string, size_t length){
    token_free_string(self);
    self->length = length;
    self->string = string;
}

void token_appendStringf( Token self , char * format, ...){
    char * buffer;
    int buffer_size=0;
    int length = -1;
    
    va_list va;
    
    va_start(va, format);
    length = vsnprintf(NULL,0,format,va);
    va_end(va);
    
    while( length < 0 ){
        buffer_size +=1000;
        buffer = malloc( buffer_size );
        va_start(va, format);
        length = vsnprintf(buffer,1000,format,va);
        va_end(va);
        free(buffer);
    }    
    
    self->string = realloc( self->string, self->length + length + 1);
    int orig_length  = self->length;
    self->length += length;
    
    va_start(va, format);
    vsnprintf(self->string+orig_length  , length+1,format,va);
    va_end(va);
}

void token_cpyString( Token self , char * string){
    token_free_string(self);
    self->length = strlen(string);
    self->string = malloc( self->length + 1);
    strcpy( self->string, string);
}

void token_mkString( Token self , char * string, int length){
    token_free_string(self);    
    self->string = malloc( length + 1);
    memcpy(self->string, string, length);
    self->string[length] = 0; // null terminate
    self->length = length;
}

Token token_new_string(char * string, int length){
    Token self = token_new();
    token_mkString(self, string, length);
    return self;
}



void token_appendToken( Token self, Token other){
    if (! other->string) return;
    int orig_length  = self->length;
    self->length += other->length;
    self->string = realloc( self->string, self->length +1 );
    memcpy( self->string + orig_length , other->string, other->length);
    self->string[self->length] = 0;
}

void token_setNumber( Token self, double number){
    token_free_string(self);
    self->number = number;
}

Token token_new_identifier( char * source , int length){
    Token self = token_new();
    token_mkString( self, source,length);
    return self;
}

Token token_new_number( double number){
    Token self = token_new();
    token_setNumber(self, number);
    return self;
}





/* VARIABLES */

// search in 'array' with size 'length' for string in 'tok'
static int findvar(char ** array, int length, Token tok){
    int c;
    char * string = tok->string;
    for( c=0; c<length; c++){
        if( (array[c]) && (strcmp( string, array[c]) == 0) ) return c;
    }    
    return -1; // return -1 if it is not found
}


// add string in 'tok' to 'array' of length 'index'
static inline int addVar(char ** array, int * index, Token tok){
    int length = strlen(tok->string) + 1;
    char * string = malloc( length );
    memcpy( string, tok->string, length);
    array[ (*index)++ ] = string;
    return (*index)-1;
}    

// add a global variable
int var_global(State self, Token tok){
    int ret = findvar(self->globals, self->globals_index, tok);
    if (ret != -1) return ret; // variable already exists so return index
    return addVar( self->globals, &(self->globals_index), tok);
}

// add a lexical variable 
int var_lexical(State self, Token tok){
    int ret = findvar(self->lexicals, self->lexicals_index, tok);
    if (ret != -1) return ret; // variable already exists so return index
    self->lexical_num[self->lexical_num_index]++;
    self->lexical_num_total++;
    return addVar( self->lexicals, &(self->lexicals_index), tok);
}

// add a local variable
int var_local(State self, Token tok){
    int offset = self->locals_index - self->local_num[self->local_num_index];
    
    int ret = findvar(self->locals + offset, self->local_num[self->local_num_index], tok);
    if (ret != -1) return ret; // variable already exists so return index
        
    self->local_num[self->local_num_index]++;
    return addVar( self->locals, &(self->locals_index), tok);
}

int var_args(State self, Token tok){
    int offset = self->args_index - self->args_num[self->args_num_index];
    
    int ret = findvar(self->args + offset, self->args_num[self->args_num_index], tok);
    if (ret != -1) return ret; // variable already exists so return index
        
    self->args_num[self->args_num_index]++;
    return addVar( self->args, &(self->args_index), tok);    
}


Token identifier_new(State self, Token tok, int type){
    Token A = token_new();
    token_appendToken(A,tok);
    A->id_type = type;
    
    int index;
    
    switch(type){
        case GLOBAL_TOK:
            var_global(self,tok);
            break;
        case LEXICAL_TOK:
            var_lexical(self,tok);
            break;
        case ARG_TOK:
            var_args(self,tok);
                       
            break;
        case AUTO_TOK:
            /* look in globals */
            index = findvar(self->globals, self->globals_index, tok);
            if (index != -1){
                A->id_type = GLOBAL_TOK;
                break;
            }
            
            /* look in lexicals */
            index = findvar(self->lexicals, self->lexicals_index, tok);
            if (index != -1){
                A->id_type = LEXICAL_TOK;
                break;
            }
                        
            /* look in args */
            int offset = self->args_index - self->args_num[self->args_num_index];
            index = findvar(self->args + offset, self->args_num[self->args_num_index], tok);
            if (index != -1){
                A->id_type = ARG_TOK;
                break;
            }

            /* must be local */
            var_local(self,tok);
            A->id_type = LOCAL_TOK;
            break;
    }
    return A;
}

/* returns the length of a variable token  i.e.  "tempx1"  length =6 */
size_t varspn(const char * s1){
    const char * s = s1;
    char c;
    while((c = *s)){
        if(
           (c < '0')    ||
           (c > 'z' )   ||
          ((c > '9') && (c < 'A')) ||
          ((c > 'Z') && (c < '_')) ||
          ((c > '_') && (c < 'a'))
        ) break;
        s++;
    }
    return s-s1;
}




static void tokenize_string(char ** source, Token token, int * linenum){        
    /* read the start character to determine the terminating character */
    char stop;
    switch ( **source ){
        case '"':
            stop = '"';
            break;
        case '\'':
            stop = '\'';
            break;
        default:
            stop = '\0';
    }

    /* move one past the start character */
    char * pchar = *source + 1; 

    /* count the length of the string */
    size_t length = 0;
    char c;
    while( (c=*pchar) && (*pchar != stop)){

        if (c == '\\'){
            pchar++;
            length++;
        }else if (c == '\n'){
            (*linenum)++;
            length++;
        }else if (c == '\r'){
            (*linenum)++;
            length++;
        }            
        length++;
        pchar++;
    }

    /* allocate memory for string */
    char * string = malloc( length + 1);
    
    /* reset pchar to start of string */
    pchar = *source + 1; 
    
    /* copy string into token */
    size_t i = 0;
    while( (c=*pchar) && (*pchar != stop)){
    
        if (c == '\\'){
            /* add the slash to output */
            string[i++] = c;
            
            /* increment to next character and add it as well */
            pchar++;
            c = *pchar;          
            
        }else if (c == '\n'){
            string[i++] = '\\';
            c = 'n';
        
        }else if (c == '\r'){
            string[i++] = '\\';
            c = 'n';
        }
        
        string[i++] = c;
        pchar++;
    }
    
    string[length] = 0;
    
    /* adjust the current source position */
    *source = pchar +1;
    
    token_setString( token , string, length);
}


void state_output(State state, Token tok){
    
    int c;
    for (c=0; c< state->function_stack_index; c++){
        token_appendToken(tok, state->function_stack[ c ]);
        token_free( state->function_stack[ c ] );
    }    
    state->output = tok->string;
    tok->string = NULL;
    token_free(tok);    
}


#define KEYWORD( KEY, TOK ) \
if ( memcmp( KEY , source, length) == 0 ){\
    token = token_new();\
    token->type = TOK;\
    source += length;\
    keyword=1;\
}  


#define OP( KEY, TOK ) \
if (KEY[0] == source[0]){ \
    token = token_new();\
    token->type = TOK;\
    source ++;\
}  

#define OP2( KEY, TOK ) \
if ((KEY[0] == source[0])&&(KEY[1] == source[1]))   { \
    token = token_new();\
    token->type = TOK;\
    source +=2;\
}


#include "grammer.c"



char * compile_cstr( char * sourcecode){
    
    char * source = sourcecode;
    int length;
    int keyword;

    Token token;
    State state =  state_new();
    
    void * parser = ParseAlloc( malloc );
    //ParseTrace(stdout, "           ");

    
    token = token_new();
    token->type=START;
    Parse( parser, token->type, token, state); 
    
    while( *source && (! state->error) ){
        
        state->source = source;
        token = NULL;
        keyword=0;
        
        /* NEWLINE */
        if ('\n' == *source){
            state->linenum++;
            source++;
        }else if ('\r' == *source){
            state->linenum++;
            source++;
            if ('\n' == *source)source++;
        /* WHITESPACE */
        }else if (' ' == *source){
            source++;
        /* TAB */
        }else if ('\t' == *source){
            source++;			
			
        /* COMMENTS */
        }else if ('#' == *source){    
            while( *source && (*source != '\n')) {
                source++;
            }            
            
        /* IDENTIFIER */
        }else if ( isalpha(*source)|| (*source == '_') ) {
            length = varspn(source);
            
            /* Keyword */
            switch(length){
                case 2:
                    KEYWORD( "do" ,      DO  ) else;
                    KEYWORD( "or" ,      OR  ) break;
                case 3:
                    KEYWORD( "def" ,     DEF ) else
                    KEYWORD( "del" ,     DEL ) else
                    KEYWORD( "and" ,    AND ) else
                    KEYWORD( "sys" ,     SYS ) else
                    KEYWORD( "arg" ,     ARG ) break;
                case 4:
                    KEYWORD( "trap" ,    TRAP ) else;             
                    KEYWORD( "loop" ,    LOOP ) else;             
                    KEYWORD( "this" ,    THIS ) else;             
                    KEYWORD( "self" ,    SELF ) else;             
                    KEYWORD( "args" ,    ARGS ) else;             
                    KEYWORD( "func" ,    FUNC ) break;             
                case 5:
                    KEYWORD( "raise" ,   RAISE ) break;                
                case 6:
                    KEYWORD( "return" , RETURN ) else
                    KEYWORD( "global" , GLOBAL ) break; 
                case 7:
                    KEYWORD( "trapped" ,   TRAPPED ) else;
                    KEYWORD( "lexical" ,   LEXICAL ) break;
            }
            
            if ( ! keyword){
                token = token_new_identifier(source,length);
                token->type = IDENTIFIER;
                source +=length;
            }
            
        /* NUMBER */    
        }else if ((( (source[0] >='0') && (source[0] <='9') ) || 
               ( (source[0]=='.') && (source[1]) &&  ( (source[1] >='0') && (source[1] <='9') )  ))){
            
            //printf("__N\n");
            token = token_new_number( strtod(source, &(source)) );
            token->type = NUMBER;
            //printf("__N\n");
        
        /* STRING */
        }else if ( *source == '"'){
            token = token_new();
            tokenize_string(&source, token, &state->linenum);
            token->type = STRING;

        /* OPERATORS */
        }else{
            
            OP2("==", EQ) else
            OP2("!=", NE) else
            OP2("<>", CMP) else
            OP2(">=", GE) else
            OP2("<=", LE) else
            
            OP( "|" , TRAP) else
                
            OP( "!" , BANG) else
            
            OP( "&" , AMPAND) else
                
            OP( "." , DOT) else
            OP( ";" , SEMICOLON) else
            OP( ":", COLON) else
            OP( "?", QUESTION) else
            
            OP( "@", TRAPPED) else
            
            OP( "[", LBRACKET) else
            OP( "]", RBRACKET) else
            
            OP( "{", LBRACE) else
            OP( "}", RBRACE) else           
            
            OP( "(", LPAREN) else
            OP( ")", RPAREN) else
            OP( ",", COMMA) else

            OP( "+", PLUS) else
            OP( "-", MINUS) else
            OP( "*", MULTIPLY) else
            OP( "/", DIVIDE) else
            OP( "^", HAT) else
            OP( "%", PERCENT) else
            OP( "<", LT) else
            OP( ">", GT) else
            
            OP( "=", ASSIGN)else
                
            {
                state->error = malloc(1024);
                snprintf(state->error, 1024,"ERR:Syntax Error\nline: %i\nchar: '%c'\n", state->linenum, *source);
            }
    
        }
        
        if (token){
            token->linenum = state->linenum;
            //printf("__Parse_start\n");
            Parse( parser, token->type, token, state); 
            //printf("__Parse_end\n");
        }
    }
    
    /* finish parsing */
    token = NULL;
    Parse( parser, 0, NULL, state);    
    ParseFree( parser, free);
    return state_free(state);
}

char * compile_file( char * filename){
    char * sourcecode = file_load(filename);
    char * acode = compile_cstr( sourcecode );
    free( sourcecode );
    return acode;
}




