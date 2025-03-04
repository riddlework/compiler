#include "symtab.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
 
//function stubs
void add_decl(char *lexeme, Type type, int num_args, symtab_entry *next, Scope scope);
symtab_entry* lookup(char *lexeme);
symtab_entry* create_symtab_entry(char *lexeme, Scope scope, Type type, int num_args);
void add_entry(symtab_entry *entry_to_add, Scope scope);

// 0 - global
// 1 - local
symtab_entry *symtab_hds[2];
extern int line_num;
extern int chk_decl_flag;
extern Scope cur_scope;


void add_decl(char *lexeme, Type type, int num_args, symtab_entry *next, Scope scope) {
    if (chk_decl_flag) {
        // lockup the lexeme in the stack of scopes
        symtab_entry *entry = lookup(lexeme);

        if (entry) {
            // if function, throw error
            // if variable, only throw error if scopes and types match
            // double declaration -- throw error
            if (entry->type == FUNC || (entry->type == type && entry->scope == scope)) {
                // TODO: change way you track when the scopes are equal?
                fprintf(stderr, "SEMANTIC ERROR: LINE %d: Double declaration of [%s]\n", line_num, lexeme);
                exit(1);
            }
        } else {
            // create a new entry in the symbol table
            symtab_entry *new_entry = create_symtab_entry(lexeme, scope, type, 0);

            // TODO: keep track of the number of arguments when type == FUNC
            add_entry(new_entry, scope);
        }
    }
}

// adds an entry to the most deeply nested scope
void add_entry(symtab_entry *entry_to_add, Scope scope) {
    if (chk_decl_flag) {
        entry_to_add->next = symtab_hds[scope];
        symtab_hds[scope] = entry_to_add;
    }
}

symtab_entry* create_symtab_entry(char *lexeme, Scope scope, Type type, int num_args) {
    if (chk_decl_flag) {
        symtab_entry* new_entry = (symtab_entry *) calloc(1, sizeof(symtab_entry *));

        new_entry->lexeme = lexeme;
        new_entry->scope = scope;
        new_entry->type = type;
        new_entry->num_args = num_args;
        new_entry->next = NULL; // next entry should always be NULL

        return new_entry;
    } return NULL;
}

// perform a lookup in the symtab_entry table
symtab_entry* lookup(char *lexeme) {
    if (chk_decl_flag) {
        symtab_entry *cur = symtab_hds[cur_scope];
        if (!cur) return NULL;
        while ((cur = cur->next)) {
            if (strcmp(lexeme, cur->lexeme) == 0) return cur;
        }
    } return NULL;
}




// OLD FUNCTIONS
//function stubs
void add_symtab_entry(char *lexeme, Type type, int scope_type);
void in_table(char *lexeme, Type type, int scope_type);


// add a new symtab_entry to the symtab_entry table
void add_symtab_entry(char *lexeme, Type type, int scope_type) {
    if (chk_decl_flag) {
        // create symtab_entry struct
        symtab_entry *sym = create_symtab_entry(lexeme, type, NULL);


        // check for double declaration in the same scope
        if (lookup(sym->lexeme, sym->type, scope_type)) {
            fprintf(stderr, "DUPLICATE symtab_entry ERROR ON LINE %d.\n", line_num);
            exit(1);
        }
        
        // symtab_entry is legal -- add to symtab_entry table
        push_symtab_entry(sym, scope_type);
    }
}

void in_table(char *lexeme, Type type, int scope_type) {
    if (chk_decl_flag) {
        switch (type) {
            case FUNC:
                // search global table
                if (!lookup(lexeme, type, GLOBAL)) {
                    fprintf(stderr, "CALL BEFORE USE ERROR ON LINE %d.\n", line_num);
                    /*dump_table(GLOBAL);*/
                    /*dump_table(LOCAL);*/
                    exit(1);
                }
                break;
            case INT:
                break;
        }
    }
}

