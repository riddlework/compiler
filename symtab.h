/*
 * File: parser.h
 * Author: Maria Fay Garcia
 * Purpose: To outline type definitions for parsing
 *          and for semantic checking.
 */
#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// struct definitions
typedef enum scope {
    GLOBAL,
    LOCAL
} Scope;

typedef enum type {
    FUNC,
    VAR,
    PARAM
} Type;

typedef struct symtab_entry {
           char          *lexeme;
           Type             type;
           Scope           scope;
    struct symtab_entry    *next;

           // for functions
           int          num_args; // the number of arguments the function requires
           int   num_args_passed; // the number of arguments passed to the function

           // for code gen
           int         fp_offset;
           int          is_param;
           // params +8 from fp
           // locals -4 from fp
} symtab_entry;

#include "ast.h"

// function stubs
void          perform_semantic_checking(ASTNode *node   );
int           count_num_args           (ASTNode *node   );
int           count_num_args_passed    (ASTNode *node   );
symtab_entry* add_decl                 (char    *lexeme  ,
                                        Type     type   );
symtab_entry* add_entry                (char    *lexeme  ,
                                        Type     type   );
symtab_entry* symtab_lookup            (char    *lexeme );
symtab_entry* scope_lookup             (char    *lexeme  ,
                                        Scope    scope  );
void          throw_error              (int      line_num,
                                        int      col_num ,
                                        char    *err_msg , 
                                        Type     type    ,
                                        char    *lexeme );
void dump_symtab();

// globals
extern int        line_num;
extern int         col_num;
extern int   chk_decl_flag;
extern Scope     cur_scope;

#endif  /* __SYMTAB_H__ */
