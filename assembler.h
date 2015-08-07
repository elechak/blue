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

#ifndef _ASSEMBLER_
#define _ASSEMBLER_


#include "bytes.h"
#include "library_header.h"
#include "stream.h"
#include "stack.h"
#include "bstring.h"

Bytes assemble(char * source);
string_t disassemble(unsigned char * bytecode);


enum op_id{
    NO_OP,
    INTRO,
    LABEL,
    ENTRY,
    PUSH_SYS,
    PUSH_MODULE,
    PUSH_BLOCK, 
    PUSH_ARG,
    PUSH_LEX,
    PUSH_ARGS,
    PUSH_THIS,
    PUSH_SELF,    
    PUSH_VAR,
    PUSH_GVAR,
    PUSH_NUM,
    PUSH_STR,    
    PUSH_ARRAY,
    PUSH_TRAPPED,        
    PUSH_NULL,
    PUSH_DEF,
    LINE,
    JUMP,
    JIT,
    JIF,
    JIF_POP,
    JINC,
    POP,
    ALLOC_MODULE,
    FREE_MODULE,
    ALLOC_LOCAL,
    FREE_LOCAL,
    ALLOC_LEXICAL,
    TYPEOF,    
    STORE_ARG,
    STORE_LEX,
    STORE_CHILD,
    STORE_ATTR,
    STORE_GVAR,
    STORE_VAR,
    LT,
    GT,
    EQ,
    NE,
    LE,
    GE,
    CMP,
    NOT,
    AND,    
    OR, 
    TEST,
    ELSE,
    TRAP,
    NEG,       
    ADD,
    SUB,
    MULT,
    DIV,
    MOD,
    POW,
    GET_ATTR,
    GET_CHILD,
    DO,
    CALL,
    CALL_CHILD,
    CALL_ATTR,
    DEL_ATTR,   
    DEL_CHILD,       
    DEF,
    RETURN,
    RAISE, 
    PAUSE,
    STOP,
    CLEAR,
    END,    
    MARKER,    
};


#endif

