

#include "assembler.h"
#include "compiler.h"

typedef double number_t;



#define OP(NAME,LEFT,RIGHT)  [NAME] = {#NAME , LEFT , RIGHT},
struct{
    char name[16];
    int left;
    int right;
} op_info[] = {
    OP( NO_OP         ,     0 ,   0)
    OP( INTRO         ,     0 ,   0)
    OP( LABEL         ,     0 ,   0)
    [ENTRY] ={"LABEL" ,     0 ,   0},
    OP( PUSH_SYS      ,     0 ,   0)
    OP( PUSH_MODULE   ,     0 ,   0)
    OP( PUSH_BLOCK    ,     0 ,   0)
    OP( PUSH_VAR      ,     0 ,   0)
    OP( PUSH_GVAR     ,     0 ,   0)
    OP( PUSH_ARG      ,     0 ,   0)
    OP( PUSH_LEX      ,     0 ,   0)
    OP( PUSH_ARGS     ,     0 ,   0)
    OP( PUSH_THIS     ,     0 ,   0)
    OP( PUSH_SELF     ,     0 ,   0)
    OP( PUSH_NUM      ,     0 ,   0)
    OP( PUSH_STR      ,     0 ,   0)
    OP( PUSH_ARRAY    ,     0 ,   0)
    OP( PUSH_TRAPPED  ,     0 ,   0)
    OP( PUSH_NULL     ,     0 ,   0)
    OP( PUSH_DEF      ,     0 ,   0)
    OP( LINE          ,     0 ,   0)
    OP( PAUSE         ,     0 ,   0)
    OP( STOP          ,     0 ,   0)
    OP( CLEAR         ,     0 ,   0)
    OP( END           ,     0 ,   0)
    OP( JUMP          ,     0 ,   0)
    OP( JIF           ,     0 ,   0)
    OP( JIF_POP       ,     0 ,   0)
    OP( JIT           ,     0 ,   0)
    OP( JINC          ,     0 ,   0)
    OP( POP           ,     0 ,   0)
    OP( ALLOC_MODULE  ,     0 ,   0)
    OP( FREE_MODULE   ,     0 ,   0)
    OP( ALLOC_LOCAL   ,     0 ,   0)
    OP( FREE_LOCAL    ,     0 ,   0)
    OP( ALLOC_LEXICAL ,     0 ,   0)    
    OP( TYPEOF        ,     89 , 89)
    OP( STORE_ARG     ,     6 ,  12)
    OP( STORE_LEX     ,     6 ,  12)
    OP( STORE_CHILD   ,     6 ,  12)
    OP( STORE_ATTR    ,     6 ,  12)
    OP( STORE_GVAR    ,     6 ,  12)
    OP( STORE_VAR     ,     6 ,  12)
    OP( LT            ,    12 ,  12)
    OP( GT            ,    12 ,  12)
    OP( EQ            ,    12 ,  12)
    OP( NE            ,    12 ,  12)
    OP( LE            ,    12 ,  12)
    OP( GE            ,    12 ,  12)
    OP( CMP           ,    12 ,  12)
    OP( NOT           ,    40 ,  40)
    OP( AND           ,    15 ,  15)
    OP( OR            ,    15 ,  15)
    OP( TEST          ,    10 ,  11)
    OP( ELSE          ,    10 ,  11)
    OP( TRAP          ,     4 ,  49)
    OP( NEG           ,    40 ,  40)
    OP( ADD           ,    20 ,  20)
    OP( SUB           ,    20 ,  20)
    OP( MULT          ,    30 ,  30)
    OP( DIV           ,    30 ,  30)
    OP( MOD           ,    30 ,  30)
    OP( POW           ,    30 ,  31)
    OP( GET_ATTR      ,    90 ,  89)
    OP( GET_CHILD     ,    90 ,  89)
    OP( DO            ,    0 ,   0)
    OP( CALL          ,    51 ,  51)
    OP( CALL_CHILD    ,    51 ,  51)
    OP( CALL_ATTR     ,    51 ,  51)
    OP( DEL_ATTR      ,    60 ,  60)
    OP( DEL_CHILD     ,    60 ,  60)
    OP( DEF           ,     0 ,  0)
    OP( RETURN        ,     5 ,  16)
    OP( RAISE         ,     5 ,  16)
    OP( MARKER        ,     0 ,  90)
};




/* ignore whitespace and comments */
char * skip_whitespace(char * pstring){
        // skip whitespaces  (don't ignore newlines)
        while(  (*pstring && (*pstring < '!')) && (*pstring != '\n')  ){
            pstring++;
        }
        //skip comments
        if (*pstring == '#') while( *pstring ){
            pstring++;
            if (*pstring == '\n'){
                break;
            }
        }
        return pstring;
}




typedef struct Label_info{
    string_t name;
    size_t offset;
} * Label_info;


typedef struct string_backpatch_node{
    size_t offset;
    struct string_backpatch_node * next;
} * string_backpatch_node;




stack_define(label_stack_t, Label_info);
stack_define(string_t_stack, string_t);




static void eat_whitespace(const char ** s){
    while ( **s && ((**s==' ')||(**s=='\t')||(**s=='\r')||(**s=='\n'))) {
        (*s)++;
    }
}

static int scan_hex(const char ** s, char * intarray){
    int  n , c;
    unsigned int i;    
    c=0;    
    if (intarray){    
        while (**s != ';'){
            n=0;
            if (! sscanf(*s, "%x%n", &i , &n) ) break;
            intarray[c] = i;
            *s+=n;
            c++;
            eat_whitespace(s);
        }
    }else{
        while (**s != ';'){
            n=0;
            if (! sscanf(*s, "%x%n", &i, &n)) break;
            *s+=n;
            c++;
            eat_whitespace(s);
        }
    }

    return c;
}

static int scan_int(const char ** s, char * intarray){
    int  n , c;
    unsigned int i;
    c=0;    
    if (intarray){    
        while (**s != ';'){
            n=0;
            if (! sscanf(*s, "%u%n", &i , &n)) break;
            intarray[c] = i;
            *s+=n;
            c++;
            eat_whitespace(s);
        }
    }else{
        while (**s != ';'){
            n=0;
            if (! sscanf(*s, "%u%n", &i, &n))break;
            *s+=n;
            c++;
            eat_whitespace(s);
        }
    }
    return c;
}

static int scan_oct(const char ** s, char * intarray){
    int  n , c;
    unsigned int i;    
    c=0;    
    if (intarray){    
        while (**s != ';'){
            n=0;
            if (! sscanf(*s, "%o%n", &i , &n))break;
            intarray[c] = i;
            *s+=n;
            c++;
            eat_whitespace(s);
        }
    }else{
        while (**s != ';'){
            n=0;
            if (! sscanf(*s, "%o%n", &i, &n)) break;
            *s+=n;
            c++;
            eat_whitespace(s);
        }
    }
    return c;
}


size_t addStringConst(string_t_stack string_const, char * cstring){

    /* skip whitespace between command and start of string */
    cstring = skip_whitespace(cstring);

    /* read the start character to determine the string's terminating character */
    char stop = 0;
    switch (*cstring){
        case '"':
            stop = '"';
            cstring++;
            break;
        case '\'':
            stop = '\'';
            cstring++;
            break;
        default:
            stop = '\n';
            break;
    }


    /* create a pointer that will point to the current character in the string */
    const char * pchar = cstring;

    /* count the length of the string */
    size_t length = 0;
    while( *pchar != stop ){

        if (*pchar == '\\'){
            pchar++;
            switch(*pchar){
                case 'd':
                    pchar++;
                    length+= scan_int(&pchar, NULL);
                    pchar++;
                    continue;
                case 'h':
                    pchar++;
                    length += scan_hex(&pchar, NULL);
                    pchar++;
                    continue;
                case 'o':
                    pchar++;
                    length += scan_oct(&pchar, NULL);
                    pchar++;
                    continue;
            }            
        }
        length++;
        pchar++;
    }

    /* create string object to hold new constant */
    string_t str = string_new_ls(length, NULL);

    /* reset pchar to start of string */
    pchar = cstring;

    /* copy string into str */
    size_t i = 0 ;
    while( *pchar != stop ){

        if (*pchar == '\\'){
            pchar++;
            switch(*pchar){
                case 'n':
                    str->data[i++] = '\n';
                    pchar++;
                    continue;
                case 't':
                    str->data[i++] = '\t';
                    pchar++;
                    continue;
                case 'r':
                    str->data[i++] = '\r';
                    pchar++;
                    continue;
                
                
                case 'd':
                    pchar++;
                    i+= scan_int(&pchar, str->data+i);
                    pchar++;
                    continue;
                case 'h':
                    pchar++;
                    i+= scan_hex(&pchar, str->data+i);
                    pchar++;
                    continue;
                case 'o':
                    pchar++;
                    i+= scan_oct(&pchar, str->data+i);
                    pchar++;
                    continue;
            }
        }
        str->data[i++] = *pchar;
        pchar++;
    }

    /* test for redundancy */
    string_t test;
    stack_iter(string_const, test, i){
        if (string_compare(test, str)==0){
            free(str);
            return i;
        }
    }

    stack_push(string_const, str);
    return (stack_length(string_const) - 1);
}



void addLabel(label_stack_t labels, Bytes output, char * cstring){

    /* skip whitespace between command and start of string */
    cstring = skip_whitespace(cstring);

    /* read the start character to determine the string's terminating character */
    char stop = 0;
    switch (*cstring){
        case '"':
            stop = '"';
            cstring++;
            break;
        case '\'':
            stop = '\'';
            cstring++;
            break;
        default:
            stop = '\n';
            break;
    }

    /* create a pointer that will point to the current character in the string */
    const char * pchar = cstring;

    /* count the length of the string */
    size_t length = 0;
    while( *pchar != stop ){

        if (*pchar == '\\'){
            pchar++;
        }
        length++;
        pchar++;
    }

    /* create string object to hold new constant */
    string_t str = string_new_ls(length, NULL);

    /* reset pchar to start of string */
    pchar = cstring;

    /* copy string into str */
    size_t i = 0 ;
    while( *pchar != stop ){

        if (*pchar == '\\'){
            pchar++;
            switch(*pchar){
                case 'n':
                    str->data[i++] = '\n';
                    pchar++;
                    continue;
            }
        }
        str->data[i++] = *pchar;
        pchar++;
    }

    Label_info li = malloc(sizeof( *li ));
    li->name = str;
    li->offset = bytes_length(output);
    stack_push(labels, li);
}


/* searches the op_atom info and finds the integer id that matches the opcode_name */
static inline int getOpId(char * cstring, int length){
    int c;

    for (c=0; c<END; c++){
        if ( memcmp(cstring, op_info[c].name, length) == 0 ){
            return c;
        }
    }
    return c;
}



// Assemble assembly code into bytecode
Bytes assemble(char * acode){
    char * buffer;
    char * cstring;
    //char buffer[256];
    int length;
    int op_id;
    size_t index = 0;
    double d;
    string_t line;

    /* string constant storage */
    stack_init(string_const, string_t_stack, 16);

    /* variables used in the far too complex string backpatching section */
    string_backpatch_node string_backpatch      = NULL;
    string_backpatch_node string_backpatch_temp = NULL;
    string_backpatch_node string_backpatch_last = NULL;

    /* create stacks for handling jumps and matching labels */
    stack_init(labels, label_stack_t, 64);
    stack_init(jumps,  label_stack_t, 64);

    /* main storage mechanism for the output */
    Bytes output = bytes_new();

    string_t newline = string_new("\n");

    bytes_writef(output,"C 4C",INTRO, 234, 7,0,0);

    char * current = acode;
    int size;


    while(*current){
        /* get the next line */
        size = strcspn( current, "\n");
        buffer = malloc( size +16);
        memcpy( buffer, current, size+1);
        buffer[size+1] = 0;

        current += size+1;
        
        if ( memcmp( buffer, "ERROR" , 5) ==0 ){
            bytes_clear(output);
            bytes_writef(output,"C 4C",INTRO, 234, 7,1,0);
            bytes_writef(output,"s",buffer);
            goto error;
        }

        /* get the opcode and return its numerical value */
        cstring = buffer;
        cstring = skip_whitespace(cstring);
        length = varspn(cstring);
        op_id = getOpId(cstring,length);

        //printf("%i %i %s", op_id,length, cstring);
        
        cstring += length;

        /* write the bytecode equivalent of the opcode */
        switch (op_id){
            case LABEL:
                /* mark the current location in the label stack */
                addLabel(labels,output, cstring);
                //printf("XXX\n");
                break;

            case DO:
            case PUSH_BLOCK:
            case JUMP:
            case JIF:
            case JIF_POP:
            case JIT:
            case JINC:
                /* write the opcode and prepare for a reference backpatch */
                bytes_writef(output,"C", op_id);
                addLabel(jumps, output,cstring);
                bytes_writef(output,"z", index);
                break;


            case PUSH_STR:
                index = addStringConst(string_const, cstring);

                bytes_writef(output,"C", op_id);
                /* add the strings index to the backpatch linked list */
                    string_backpatch_temp = malloc(sizeof(*string_backpatch_temp));
                    string_backpatch_temp->offset = bytes_length(output);
                    string_backpatch_temp->next = string_backpatch;
                    string_backpatch = string_backpatch_temp;
                bytes_writef(output,"z", index);
                break;

            case PUSH_NUM:
                cstring = skip_whitespace(cstring);
                d = strtod(cstring, NULL);
                bytes_writef(output,"C d", op_id, d);
                break;

            case PUSH_ARRAY:
            case ALLOC_LOCAL:
            case ALLOC_LEXICAL:
            case ALLOC_MODULE:
            case PUSH_ARG:
            case PUSH_LEX:
            case PUSH_GVAR:
            case PUSH_VAR:
            case STORE_ARG:
            case STORE_LEX:
            case STORE_GVAR:
            case STORE_VAR:
            case LINE:
                cstring = skip_whitespace(cstring);
                index = strtol(cstring, NULL,10);
                bytes_writef(output,"C z", op_id, index);
                break;

            default:
                bytes_writef(output,"C", op_id);
                break;
        }
        
        free(buffer);

    }

    /* Write the opcode that signals the end of the code section */
    bytes_writef(output,"C", PAUSE);

    /* STRING CONSTANTS & BACKPATCHING */
    size_t * pstring_index;
    stack_iter(string_const, line, index){
        string_backpatch_temp = string_backpatch;
        string_backpatch_last = NULL;


        /* loop through all of the strings that need to be backpatched */
        while(string_backpatch_temp){

            /* pstring_index is a pointer to the PUSH_STR argument, currently it holds the index of the desired string in the string_const_stack */
            pstring_index = bytes_atOffset(output,string_backpatch_temp->offset);

            /* the current string matches the index of the string that needs to be backpatched */
            if ( *pstring_index == index){

                /* set the value of pstring_index to the position in the bytecode, instead of the temporary index currently stored there */
                *pstring_index = bytes_length(output);

                /* delete resolved backpatch */
                if (string_backpatch_last){
                    string_backpatch_last->next = string_backpatch_temp->next;
                    free(string_backpatch_temp);
                    string_backpatch_temp = string_backpatch_last->next;

                }else{
                    string_backpatch = string_backpatch_temp->next;
                    free(string_backpatch_temp);
                    if (string_backpatch){
                        string_backpatch_temp = string_backpatch;
                    }else{
                        break;
                    }
                }


                continue;
            }


            string_backpatch_last = string_backpatch_temp;
            string_backpatch_temp = string_backpatch_temp->next;
        }

        bytes_writef(output,"zs", line->length, line->data);
    }


    Label_info label;
    Label_info jump;
    size_t index2;

    /* JUMPS */
    stack_iter(jumps, jump, index){
        stack_iter(labels, label, index2){
            if ( string_compare(label->name, jump->name) == 0){
                pstring_index = bytes_atOffset(output,jump->offset);
                *pstring_index = label->offset;
                break;
            }
        }
    }

    error:

    free(newline);

    /* free the label stack */
    stack_iter(labels, label, index2){
        free(label->name);
        free(label);
    }
    stack_free(labels);

    /* free the jump stack */
    stack_iter(jumps, jump, index2){
        free(jump->name);
        free(jump);
    }
    stack_free(jumps);

    /* free the string constants */
    stack_iter(string_const, line, index2){
        free(line);
    }
    stack_free(string_const);


    return output;
}









/* ---------------------------------------- DISASSEMBLE -------------------------------------------- */

#define ADDRESS      (current-bytecode)
#define GETOFFSET   *((size_t*)current)

#define CONCAT stack_push(str_stack, string_new(buffer));

#define BUFFERSIZE 63


#define DIS_START  \
    i = snprintf(buffer,BUFFERSIZE, "%-6li   %.3u:%-16s" ,ADDRESS ,*current, op_info[*current].name);\
    current++;

#define DIS_LONG  \
    i += snprintf(buffer+i,BUFFERSIZE-i, "  %-5li" ,*((long*)current));\
    current+=sizeof(long);

#define DIS_SIZE  \
    i += snprintf(buffer+i,BUFFERSIZE-i, "  %-5li" ,*((size_t*)current));\
    current+=sizeof(size_t);

#define DIS_NUMBER  \
    i += snprintf(buffer+i,BUFFERSIZE-i, "  (%g)" ,*((number_t*)current));\
    current+=sizeof(number_t);

#define DIS_STRING  \
    s = (string_t)(bytecode + *((size_t*)current) );\
    i += snprintf(buffer+i,BUFFERSIZE-i, "  '%.*s'" ,(int)s->length,s->data);\
    snprintf(buffer+i,BUFFERSIZE - i, "\n" );\
    stack_push(str_stack, string_new(buffer));\
    i = snprintf(buffer,BUFFERSIZE, "%-5li '%.*s'" ,*((size_t*)current), (int)s->length, s->data);\
    current+=sizeof(size_t);\

#define DIS_END  \
    snprintf(buffer+i,BUFFERSIZE - i, "\n" );\
    stack_push(str_stack, string_new(buffer));

static void _disassemble(string_t_stack str_stack, unsigned char * bytecode){

    char buffer[BUFFERSIZE+1];
    string_t s = NULL;
    int i;

    unsigned char * current = bytecode;

    while(1){
        switch(*current){

            case INTRO:
                DIS_START
                DIS_END
                current += 4;
                break;

            case ALLOC_MODULE:
            case ALLOC_LOCAL:
            case ALLOC_LEXICAL:
            case LINE:
            case PUSH_ARG:
            case DO:
            case PUSH_BLOCK:
            case PUSH_ARRAY:
            case PUSH_GVAR:
            case PUSH_LEX:
            case PUSH_VAR:
            case STORE_ARG:
            case STORE_VAR:
            case STORE_GVAR:
            case STORE_LEX:
            case JIT:
            case JIF:
            case JIF_POP:
            case JUMP:
            case JINC:
                DIS_START
                DIS_SIZE
                DIS_END
                break;

            case PUSH_NUM:
                DIS_START
                DIS_NUMBER
                DIS_END
                break;

            case PUSH_STR:
                DIS_START
                DIS_STRING
                DIS_END
                break;

            case PAUSE:
                return;

            default:
                DIS_START
                DIS_END
                break;
        }
    }
    printf("\n");
}


static int decomp_cmp(const void * a, const void * b){
    long la, lb;
    la = strtol( (*( (string_t *) a ))->data , NULL, 10);
    lb = strtol( (*( (string_t *) b ))->data , NULL, 10);
    return la - lb;
}


string_t disassemble(unsigned char * bytecode){

    stack_init(str_stack, string_t_stack, 32);
    _disassemble(str_stack , bytecode);
    qsort(str_stack->data, stack_length(str_stack), sizeof(string_t), decomp_cmp);

    string_t sum = string_new("");
    string_t temp = NULL;

    string_t str = NULL;
    string_t last = NULL;
    int c;
    stack_iter(str_stack, str, c){
        if (  last && (strcmp(last->data,str->data)==0) ){
            continue;
        }
        last = str;
        temp = sum;
        sum = string_new_add(sum , str);
        free(temp);
    }

    stack_iter(str_stack, str, c) free(str);
    stack_free(str_stack);

    return sum;
}


