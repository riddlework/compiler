#include "symtab.h"
 
symtab_entry *symtab_hds[2];
char *types[2] = {"GLOBAL", "LOCAL"};
char *err_types[2] = {"SYNTAX", "SEMANTIC"};


// adds a declaration to the symbol table or throws an error if necessary
symtab_entry* add_decl(const char *func, char *lexeme, Type type, symtab_entry* func_defn) {
    if (chk_decl_flag) {
        symtab_entry *entry = symtab_lookup(lexeme);

        if (entry && entry->scope == cur_scope) {
            // double declaration, throw error
            throw_error("Double declaration of", __func__, type, lexeme);
        } else {
            // create a new entry in the symbol table -- add it to the desired scope
            symtab_entry *new_entry = create_and_add_entry(lexeme, type);

            // increment parent func num of args if necessary
            if (func_defn) func_defn->num_args++;

            // return a pointer to the new entry
            return new_entry;
        }
    } return NULL;
}

// create a new symtab entry, add it to the desired scope, and return it
symtab_entry* create_and_add_entry(char *lexeme, Type type) {
    if (chk_decl_flag) {
        symtab_entry* new_entry = (symtab_entry *) calloc(1, sizeof(symtab_entry));

        new_entry->lexeme = lexeme;
        new_entry->type = type;
        // num_args should be 0
        // num_args_passed should be 0
        new_entry->scope = cur_scope;
        // next should be NULL

        // add entry to the desired scope
        new_entry->next = symtab_hds[cur_scope];
        symtab_hds[cur_scope] = new_entry;
        return new_entry;
    } return NULL;
}

// return most deeply nested instance of lexeme
symtab_entry *symtab_lookup(char *lexeme) {
    symtab_entry *to_return = scope_lookup(lexeme, LOCAL);
    if (!to_return) to_return = scope_lookup(lexeme, GLOBAL);
    return to_return;
}

// perform a lookup for a particular scope in the symbol table
symtab_entry* scope_lookup(char *lexeme, Scope scope) {
    if (chk_decl_flag) {
        symtab_entry *cur = symtab_hds[scope];
        if (!cur) return NULL;
        while (cur) {
            if (strcmp(lexeme, cur->lexeme) == 0) return cur;
            cur = cur->next;
        }
    } return NULL;
}

// print an err msg to stderr and exit the program
void throw_error(char *err_msg, const char *func, Type type, char *lexeme) {
    fprintf(stderr,
            "SEMANTIC ERROR IN LINE %d [%s] %s %s [%s]\n",
            line_num,
            func,
            err_msg,
            types[type],
            lexeme);
    exit(1);
}

void dump_symtab() {
    char *types[2] = {"FUNC", "VAR"};
    char *scopes[2] = {"GLOBAL", "LOCAL"};

    symtab_entry *cur = symtab_hds[LOCAL];

    printf("--------- GLOBAL SCOPE ----------\n");
    cur = symtab_hds[GLOBAL];
    while (cur) {
        printf("-------------\n%s\n%s\n%d\n%s\n------------\n",
                cur->lexeme,
                types[cur->type],
                cur->num_args,
                scopes[cur->scope]);
        cur = cur->next;
    }

    printf("--------- LOCAL SCOPE -----------\n");
    while (cur) {
        printf("-------------\n%s\n%s\n%d\n%s\n------------\n",
                cur->lexeme,
                types[cur->type],
                cur->num_args,
                scopes[cur->scope]);
        cur = cur->next;
    }
}

