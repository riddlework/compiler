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

// TODO: change INT to VAR, change type to identifier_type?
typedef enum type {
    FUNC,
    VAR
} Type;

typedef struct symtab_entry {
           char          *lexeme;
           Type             type;
           int          num_args; // the number of arguments the function requires
           int num_args_passed; // the number of arguments passed to the function
           Scope           scope;
    struct symtab_entry    *next;
} symtab_entry;


// function stubs
symtab_entry* add_decl(const char *func, char *lexeme, Type type, symtab_entry* func_defn);
symtab_entry* create_and_add_entry(char *lexeme, Type type);
symtab_entry* symtab_lookup(char *lexeme);
symtab_entry* scope_lookup(char *lexeme, Scope scope);
void throw_error(char *err_msg, const char *func, Type type, char *lexeme);

void dump_symtab();

// globals
extern int line_num;
extern int chk_decl_flag;
extern Scope cur_scope;

#endif  /* __SYMTAB_H__ */
