/*
 * File: parser.h
 * Author: Maria Fay Garcia
 * Purpose: To outline type definitions for parsing
 *          and for semantic checking.
 */
#ifndef __SYMTAB_H__
#define __SYMTAB_H__

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
           int          num_args;
    struct symtab_entry    *next;
           Scope           scope;
} symtab_entry;


// function stubs
void add_decl(char *lexeme, Type type, int num_args, symtab_entry *next, Scope scope);
symtab_entry* lookup          (char *lexeme);
symtab_entry* create_symtab_entry(char *lexeme, Scope scope, Type type, int num_args, symtab_entry *next);
void add_entry(symtab_entry *entry_to_add, Scope scope);

#endif  /* __SYMTAB_H__ */
