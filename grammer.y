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

%include {    
    #include "grammer.h"
    #include "assert.h"

    void add_info( State state, Token tok){
        if (state->linenum > state->debug_linenum){
            token_appendStringf(tok, "LINE        %i\n", state->linenum);
            state->debug_linenum = state->linenum;
        }
    }
    
}  

%token_type {Token}
%default_type {Token}
%token_destructor { 
    token_free($$); 
}



%destructor child { token_free($$); }
%destructor null_statement { token_free($$); }
%destructor expr_statement { token_free($$); }
%destructor attr { token_free($$); }
%destructor identifier { token_free($$); }
%destructor array { token_free($$); }
%destructor array_start { token_free($$); }
%destructor function_call_start { token_free($$); }
%destructor attr_call_start { token_free($$); }
%destructor child_call_start { token_free($$); }
%destructor argument_list { token_free($$); }
%destructor expr { token_free($$); }
%destructor bin_op { token_free($$); }
%destructor program { token_free($$);state->linenum++;}

%syntax_error { 
    state->error = malloc(1024);
    snprintf(state->error, 1024,"ERR:Syntax Error\nline: %i\nsource: '%.5s'\n", state->linenum, state->source);    
}

%extra_argument { State state }

%nonassoc START.
%nonassoc FUNC DO LOOP.
%nonassoc SEMICOLON.
%nonassoc DEF.
%nonassoc COMMA.
%nonassoc THIS SELF ARGS SYS.
%left LOW.
%left RETURN RAISE.
%right ASSIGN.
%left QUESTION COLON .
%left AND OR.
%left LT GT EQ CMP GE LE NE BANG.
%left PLUS MINUS.
%left MULTIPLY DIVIDE PERCENT.
%right HAT.
%left LPAREN RPAREN.
%left DEL.
%left LBRACKET RBRACKET LBRACE RBRACE.
%left DOT AMPAND TRAP.
%left LEXICAL GLOBAL ARG.
%left HIGH.





/* END REDUCTION */
program ::= expr_statement(A).  {
    
    token_appendStringf(A, "END\n");
    
    Token out = token_new();
    
    int globals = state_end_globals(state);
    if (globals)
        token_appendStringf( out,                                "ALLOC_MODULE   %i\n", globals);
    
    int lexical_num     =  state_end_lexicals(state);
    if (lexical_num)
        token_appendStringf(out,                                 "ALLOC_LEXICAL  %i\n", lexical_num);    
    
    int local_num        =  state_end_locals(state);
    if (local_num)
        token_appendStringf(out,                                 "ALLOC_LOCAL    %i\n", local_num);
    
    state_end_args(state);
    
    token_appendToken(out, A);
    state_output( state, out);
    token_free(A);
}

expr(A) ::= DEF expr(B).  {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf( A,          "DEF\n");
                                token_free(B);
}




/* FUNCTION DECLARATIONS */

function_dec_start ::= FUNC LBRACE.{
    state_start_function(state); 
}

expr(A) ::= function_dec_start expr_statement(B) RBRACE.  {
                                A = token_new();
                                Token function = token_new();
    
                                int function_label =  state_add_function(state, function);
    
                                int lexical_num     =  state_end_lexicals(state);
                                int local_num        =  state_end_locals(state);
                                                                     state_end_args(state);
    
                                token_appendStringf(A,           "PUSH_BLOCK     %i\n", function_label  );
                                
                                token_appendStringf(function,    "LABEL          %i\n", function_label);
    
                                if (lexical_num)
                                    token_appendStringf(function,"ALLOC_LEXICAL  %i\n", lexical_num);
                                if (local_num)
                                    token_appendStringf(function,"ALLOC_LOCAL    %i\n", local_num);
                                
                                token_appendToken(function, B);
                                token_appendStringf(function,    "END\n");
                                token_free(B);                                
}

expr(A) ::= function_dec_start  RBRACE.  {
                                A = token_new();
                                Token function = token_new();
    
                                int function_label =  state_add_function(state, function);
    
                                state_end_lexicals(state);
                                state_end_locals(state);
                                state_end_args(state);
    
                                token_appendStringf(A,           "PUSH_BLOCK     %i\n", function_label  );
                                token_appendStringf(function,    "LABEL          %i\n", function_label);
                                token_appendStringf(function,    "END\n");
}


/* DO */
expr(A) ::= DO LBRACE expr_statement(B) RBRACE.{
                                A = token_new();
    
                                Token function = token_new();
                                int function_label =  state_add_function(state, function);    
    
                                token_appendStringf(A,           "DO             %i\n", function_label  );
    
                                token_appendStringf(function,    "LABEL          %i\n", function_label);
                                token_appendToken(function, B);
                                token_appendStringf(function,    "END\n");
                                token_free(B);    
}

expr(A) ::= DO LBRACE RBRACE.{
                                A = token_new();
                                Token function = token_new();
                                int function_label =  state_add_function(state, function);    
                                token_appendStringf(A,           "DO             %i\n", function_label  );
                                token_appendStringf(function,    "LABEL          %i\n", function_label);
                                token_appendStringf(function,    "END\n");
}


expr(A) ::= LBRACE expr_statement(B) RBRACE.{
                                A = token_new();
    
                                Token function = token_new();
                                int function_label =  state_add_function(state, function);    
    
                                token_appendStringf(A,           "DO             %i\n", function_label  );
                                token_appendStringf(function,    "LABEL          %i\n", function_label);
                                token_appendToken(function, B);
                                token_appendStringf(function,    "END\n");
                                token_free(B);    
}

expr(A) ::= LBRACE  RBRACE.{
                                A = token_new();
                                Token function = token_new();
                                int function_label =  state_add_function(state, function);    
                                token_appendStringf(A,           "DO             %i\n", function_label  );    
                                token_appendStringf(function,    "LABEL          %i\n", function_label);
                                token_appendStringf(function,    "END\n");
}

/* LOOP */
expr(A) ::= LOOP LBRACE expr_statement(B) RBRACE.{
                                A = token_new();
    
                                Token function = token_new();
                                int function_label =  state_add_function(state, function);    
    
                                token_appendStringf(A,           "DO             %i\n", function_label  );
                                token_appendStringf(function,    "LABEL          %i\n", function_label);
                                token_appendToken(function, B);
                                token_appendStringf(function,    "JUMP           %i\n", function_label);
                                token_free(B);    
}

expr(A) ::= LOOP LBRACE  RBRACE.{
                                A = token_new();
                                Token function = token_new();
                                int function_label =  state_add_function(state, function);    
                                token_appendStringf(A,           "DO             %i\n", function_label  );
                                token_appendStringf(function,    "LABEL          %i\n", function_label);
                                token_appendStringf(function,    "JUMP           %i\n", function_label);
}


expr_statement(A) ::= expr_statement(B) identifier SEMICOLON. [HIGH]{
                                A=token_new();
                                token_appendToken(A,B);
}

expr_statement(A) ::= START. [HIGH]{
                                A=token_new();
}

/* MULTIPLE-LINE REDUCTION */
expr_statement(A) ::= expr(B) SEMICOLON.     [HIGH]   {
                                A = token_new();
                                token_appendToken(A, B);
                                token_appendStringf(A,           "POP\n");
                                token_free(B);    
}

expr(A) ::= expr_statement(B) expr(C).          [HIGH]   {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_free(B);    
                                token_free(C);    
}





/* BRACKETS */
array_start(A) ::= LBRACKET.  {
                                state_add_arguments(state);
                                A = token_new();
}

array(A) ::= array_start(B) RBRACKET.  {
                                state_pop_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf( A,          "PUSH_ARRAY     0\n" );   
                                token_free(B);    
}

array(A) ::= array_start(B) expr(C) RBRACKET. {
                                state_pop_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf( A,          "PUSH_ARRAY     1\n" );   
                                token_free(B);    
                                token_free(C);    
}

array(A) ::= array_start(B)  argument_list(C) RBRACKET.  {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf( A,          "PUSH_ARRAY     %i\n", state_pop_arguments(state)+1 );    
                                token_free(B);    
                                token_free(C);    
}

expr(A) ::= array(B).  [LOW]{
                                A = token_new();
                                token_appendToken(A,B);
                                token_free(B);    
}



child(A) ::= DOT array(B). [LOW]{
                                A = token_new();
                                token_appendStringf( A,          "PUSH_DEF\n" );   
                                token_appendToken(A,B);
                                token_free(B);    
}


child(A) ::= expr(B) array(C). [LOW]{
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_free(B);    
                                token_free(C);    
}


expr(A) ::= child(B).  [LOW]{
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf( A,          "GET_CHILD\n" );       
                                token_free(B);    
}


expr(A) ::= DEL child(B). {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf( A,          "DEL_CHILD\n" );
                                token_free(B);    
}



expr(A) ::= child(B) ASSIGN expr(C). {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf( A,          "STORE_CHILD\n" );   
                                token_free(B);    
                                token_free(C);    
}


child_call_start(A) ::= child(B) LPAREN.  [HIGH] {
                                state_add_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_free(B);    
}



expr(A) ::= child_call_start(B)  expr(C) RPAREN.  {
                                state_pop_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf( A,          "PUSH_ARRAY     1\n" );    
                                token_appendStringf( A,          "CALL_CHILD\n" );    
                                token_free(B);    
                                token_free(C);    
}

expr(A) ::= child_call_start(B) RPAREN.{
                                state_pop_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf( A,          "PUSH_ARRAY     0\n" );    
                                token_appendStringf( A,          "CALL_CHILD\n" ); 
                                token_free(B);    
}


expr(A) ::= child_call_start(B)  argument_list(C) RPAREN.  {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf( A,          "PUSH_ARRAY     %i\n", state_pop_arguments(state)+1 );    
                                token_appendStringf( A,          "CALL_CHILD\n" );    
                                token_free(B);    
                                token_free(C);    
}


/* PARENS */
expr(A) ::= LPAREN  RPAREN.  {
                                A = token_new();
                                token_appendStringf( A,          "PUSH_NULL\n" );    
}

expr(A) ::= LPAREN expr(B) RPAREN.  {
                                A = token_new();
                                token_appendToken(A,B);
                                token_free(B);       
}


function_call_start(A) ::= expr(B) LPAREN.  {
                                state_add_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_free(B);   
}

// no argument calls
expr(A) ::= attr_call_start(B) RPAREN.{
                                state_pop_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf( A,          "PUSH_ARRAY     0\n" );    
                                token_appendStringf( A,          "CALL_ATTR\n" ); 
                                token_free(B);    
}

expr(A) ::= function_call_start(B) RPAREN.{
                                state_pop_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf( A,          "PUSH_ARRAY     0\n" );    
                                token_appendStringf( A,          "CALL\n" ); 
                                token_free(B);    
}


// call - single argument
expr(A) ::= function_call_start(B)  expr(C) RPAREN.  {
                                state_pop_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf( A,          "PUSH_ARRAY     1\n" );    
                                token_appendStringf( A,          "CALL\n" );    
                                token_free(B);    
                                token_free(C);        
}

expr(A) ::= attr_call_start(B)  expr(C) RPAREN.  {
                                state_pop_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf( A,          "PUSH_ARRAY     1\n" );    
                                token_appendStringf( A,          "CALL_ATTR\n" );    
                                token_free(B);    
                                token_free(C);    
}


// Arguments for functions
argument_list(A) ::= expr(B) COMMA expr(C).{
                                state_inc_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_free(B);    
                                token_free(C);    
}

argument_list(A) ::= argument_list(B) COMMA expr(C).{
                                state_inc_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_free(B);    
                                token_free(C);    
}

// Functions with more than 1 argument
expr(A) ::= function_call_start(B)  argument_list(C) RPAREN.  {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf( A,          "PUSH_ARRAY     %i\n", state_pop_arguments(state)+1 );    
                                token_appendStringf( A,          "CALL\n" );    
                                token_free(B);    
                                token_free(C);    
}

expr(A) ::= attr_call_start(B)  argument_list(C) RPAREN.  {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf( A,          "PUSH_ARRAY     %i\n", state_pop_arguments(state)+1 );    
                                token_appendStringf( A,          "CALL_ATTR\n" );    
                                token_free(B);    
                                token_free(C);    
}


/* RETURN and RAISE */
expr(A) ::= RETURN. {
                                A = token_new();
                                token_appendStringf( A,          "PUSH_NULL\n" );    
                                token_appendStringf( A,          "RETURN\n" );    
}

expr(A) ::= RAISE. {
                                A = token_new();
                                token_appendStringf( A,          "PUSH_NULL\n" );    
                                token_appendStringf( A,          "RAISE\n" );    
}

expr(A) ::= RETURN expr(B). {
                                A = token_new();
                                token_appendToken( A, B );    
                                token_appendStringf( A,          "RETURN\n" );    
                                token_free(B);    
}

expr(A) ::= RAISE expr(B). {
                                A = token_new();
                                token_appendToken( A, B );    
                                token_appendStringf( A,          "RAISE\n" );    
                                token_free(B);    
}



/* Assignment */
expr(A) ::= identifier(B) ASSIGN  expr(C).   { 
                                A = token_new();
                                token_appendToken( A, C);
    
                                switch(B->id_type){
                                    case GLOBAL_TOK:
                                        token_appendStringf( A,  "STORE_GVAR     %i\n",  var_global(state,B) ); 
                                        break;
                                    case LEXICAL_TOK:
                                        token_appendStringf( A,  "STORE_LEX      %i\n",  var_lexical(state,B) ); 
                                        break;
                                    case LOCAL_TOK:
                                        token_appendStringf( A,  "STORE_VAR      %i\n",  var_local(state,B) ); 
                                        break;
                                    case ARG_TOK:
                                        token_appendStringf( A,  "STORE_ARG      %i\n",  var_args(state,B) ); 
                                        break;
                                }    
                                token_free(B);                                
                                token_free(C);                                
}

/* UNARY OPS */
expr(A) ::= MINUS expr(B).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendStringf(A,           "NEG\n");
                                token_free(B);    
}

expr(A) ::= BANG expr(B).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendStringf(A,           "NOT\n");
                                token_free(B);
}

// Binary Operators
expr(A) ::= expr(B) PLUS expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "ADD\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) MINUS expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "SUB\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) MULTIPLY expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "MULT\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) DIVIDE expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "DIV\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) HAT expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "POW\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) PERCENT expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "MOD\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) EQ expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "EQ\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) LT expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "LT\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) GT expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "GT\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) LE expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "LE\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) GE expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "GE\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) CMP expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "CMP\n");
                                token_free(B);
                                token_free(C);    
}

expr(A) ::= expr(B) NE expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendToken( A, C);
                                token_appendStringf(A,           "NE\n");
                                token_free(B);
                                token_free(C);
}


expr(A) ::= expr(B) AND expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendStringf( A,          "JIF            %i\n",  state_push_cond(state) );
                                token_appendStringf( A,          "POP\n");
                                token_appendToken( A, C);
                                token_appendStringf( A,          "LABEL          %i\n",  state_pop_cond(state) ); 
                                token_free(B);    
                                token_free(C);    
}

expr(A) ::= expr(B) OR expr(C).  { 
                                A = token_new();
                                token_appendToken( A, B);
                                token_appendStringf( A,          "JIT            %i\n",  state_push_cond(state) );
                                token_appendStringf( A,          "POP\n");
                                token_appendToken( A, C);
                                token_appendStringf( A,          "LABEL          %i\n",  state_pop_cond(state) ); 
                                token_free(B);    
                                token_free(C);    
}


expr(A) ::= expr(B) TRAP expr(C).{
                                A = token_new();
                                token_appendToken(A, B);
                                token_appendStringf( A,          "JINC           %i\n",  state_push_cond(state) );
                                token_appendToken(A, C);
                                token_appendStringf( A,          "LABEL          %i\n",  state_pop_cond(state) ); 
                                token_free(B);    
                                token_free(C);    
}

/* Conditional */

expr(A) ::=  expr(B) QUESTION expr(C) COLON expr(D). [ASSIGN]{
                                A = token_new();
                                token_appendToken(A, B);
                                token_appendStringf( A,          "JIF_POP        %i\n",  state_push_cond(state) );
                                token_appendToken(A, C);
                                token_appendStringf( A,          "JUMP           %i\n",  state_push_jump(state) );                                
                                token_appendStringf( A,          "LABEL          %i\n",  state_pop_cond(state) ); 
                                token_appendToken(A, D);
                                token_appendStringf( A,          "LABEL          %i\n",  state_pop_jump(state) ); 
                                token_free(B);    
                                token_free(C);    
                                token_free(D);    
}

expr(A) ::=  expr(B) QUESTION expr(C). [ASSIGN]{
                                A = token_new();
                                token_appendToken(A, B);
                                token_appendStringf( A,          "JIF_POP        %i\n",  state_push_cond(state) );
                                token_appendToken(A, C);
                                token_appendStringf( A,          "JUMP           %i\n",  state_push_jump(state) );                                
                                token_appendStringf( A,          "LABEL          %i\n",  state_pop_cond(state) ); 
                                token_appendStringf( A,          "PUSH_NULL\n"); 
                                token_appendStringf( A,          "LABEL          %i\n",  state_pop_jump(state) ); 
                                token_free(B);    
                                token_free(C);    
}





/* ATTRIBUTES */

attr(A) ::= DOT IDENTIFIER(B).  {
                                A = token_new();
                                add_info( state, A);
                                token_appendStringf(A ,          "PUSH_DEF\n");
                                token_appendStringf(A ,          "PUSH_STR      \"%s\"\n", B->string);
                                token_free(B);    
}    


attr(A) ::= expr(B) DOT IDENTIFIER(C).  {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf(A ,          "PUSH_STR      \"%s\"\n", C->string);
                                token_free(B);    
                                token_free(C);    
}    

expr(A) ::= DEL attr(B).{
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf(A ,          "DEL_ATTR\n");    
                                token_free(B);    
}


expr(A) ::= attr(B).  [LOW]{
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf(A ,          "GET_ATTR\n");    
                                token_free(B);
}


expr(A)  ::= attr(B) ASSIGN expr(C).{
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendToken(A,C);
                                token_appendStringf(A ,          "STORE_ATTR\n");
                                token_free(B);    
                                token_free(C);    
}

attr_call_start(A) ::= attr(B) LPAREN.  [HIGH]{
                                state_add_arguments(state);
                                A = token_new();
                                token_appendToken(A,B);
                                token_free(B);
}


/* TYPEOF Operator */
expr(A) ::= AMPAND expr(B).  {
                                A = token_new();
                                token_appendToken(A,B);
                                token_appendStringf(A ,          "TYPEOF\n");
                                token_free(B);
}


expr(A) ::= identifier(B).  [START]{
                                A = token_new();
                                add_info( state, A);
    
                                switch(B->id_type){
                                    case GLOBAL_TOK:
                                        token_appendStringf( A,  "PUSH_GVAR      %i\n",  var_global(state,B) ); 
                                        break;
                                    case LEXICAL_TOK:
                                        token_appendStringf( A,  "PUSH_LEX       %i\n",  var_lexical(state,B) ); 
                                        break;
                                    case LOCAL_TOK:
                                        token_appendStringf( A,  "PUSH_VAR       %i\n",  var_local(state,B) ); 
                                        break;
                                    case ARG_TOK:
                                        token_appendStringf( A,  "PUSH_ARG       %i\n",  var_args(state,B) ); 
                                        break;
                                }
                                token_free(B);
}







// Terminal Symbols
identifier(A) ::= ARG IDENTIFIER(B).    {
                                A = identifier_new(state, B, ARG_TOK);
                                token_free(B);
}

identifier(A) ::= GLOBAL IDENTIFIER(B).    {
                                A = identifier_new(state, B, GLOBAL_TOK);
                                token_free(B);    
}

identifier(A) ::= LEXICAL IDENTIFIER(B).   { 
                                A = identifier_new(state, B, LEXICAL_TOK);
                                token_free(B);    
}

identifier(A) ::= IDENTIFIER(B).  {
                                A = identifier_new(state, B, AUTO_TOK); 
                                token_free(B);
}

expr(A) ::= THIS.{
                                A = token_new();
                                add_info( state, A);    
                                token_appendStringf(A,           "PUSH_THIS\n");
}

expr(A) ::= SELF.{
                                A = token_new();
                                add_info( state, A);    
                                token_appendStringf(A,           "PUSH_SELF\n");
}

expr(A) ::= SYS.{
                                A = token_new();
                                add_info( state, A);    
                                token_appendStringf(A,           "PUSH_SYS\n");
}

expr(A) ::= ARGS.{
                                A = token_new();
                                add_info( state, A);
                                token_appendStringf(A,           "PUSH_ARGS\n");
}

expr(A) ::= TRAPPED.{
                                A = token_new();
                                add_info( state, A);
                                token_appendStringf(A,           "PUSH_TRAPPED\n");
}


expr(A) ::= NUMBER(B).        { 
                                A = token_new();
                                add_info( state, A);
                                token_appendStringf(A,           "PUSH_NUM       %f\n", B->number);
                                token_free(B);
}

expr(A) ::= STRING(B).        { 
                                A = token_new();
                                add_info( state, A);
                                token_appendStringf(A,           "PUSH_STR      \"%s\"\n", B->string);
                                token_free(B);
}
