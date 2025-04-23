// for performing semantic checking
#include "symtab.h"

symtab_entry *symtab_hds[2];
char *types[2] = {"GLOBAL", "LOCAL"};
char *err_types[2] = {"SYNTAX", "SEMANTIC"};

// perform semantic checking by traversing the AST for the func
void perform_semantic_checking(ASTNode *node) {

    // base case
    if (!node) return;

    // preorder traversal
    if (chk_decl_flag) {

        int line_num = node->line_num;
        int col_num  = node->col_num;
        char *lexeme = node->lexeme;

        int num_args_passed;
        symtab_entry *entry = node->symtab_entry;
        switch (node->type) {
            case FUNC_DEF:
                // count num_args by recursing through opt_formals
                // child0 is where opt_formals are stored
                node->symtab_entry->num_args = count_num_args(node->child0);
                break;
            case FUNC_CALL:
                if (!entry) {
                    // func does not exist
                    throw_error(line_num, col_num, "Use before declaration of", FUNC, lexeme);
                } else if (entry->type == VAR) {
                    // an int var tried to act as a function call
                    throw_error(line_num, col_num, "Callee is not a", FUNC, lexeme);
                }

                // verify num_args passed is correct
                // child0 is where the opt_expr_list is stored
                num_args_passed = count_num_args_passed(node->child0);
                if (num_args_passed != node->symtab_entry->num_args) {
                    throw_error(line_num, col_num, "Incorrect no. of arguments in call for", FUNC, lexeme);
                }

                break;
            case ASSG:
                if (!entry) {
                    // variable does not exist
                    throw_error(line_num, col_num, "Use before declaration of", VAR, lexeme);
                } else if (entry->type == FUNC) {
                    // trying to assign to a function
                    throw_error(line_num, col_num, "Assignment LHS is a", FUNC, lexeme);
                } break;
            case IDENTIFIER:
                // if symtab_entry is null, error
                if (!entry) {
                    throw_error(line_num, col_num, "Use before declaration of", VAR, lexeme);
                } else if (entry->type == FUNC) {
                    throw_error(line_num, col_num, "Using a function as a variable!", FUNC, lexeme);

                }

                break;
            default:
                break;
        }

        // traverse the children
        perform_semantic_checking(node->child0);
        perform_semantic_checking(node->child1);
        perform_semantic_checking(node->child2);
    }
}

// count the number of args in a func defn by recursing through formals
int count_num_args(ASTNode *node) {
    if (!node) return 0;
    else return 1 + count_num_args(node->child0);
}

// count the number of args passed to func call by recursing through exprs
int count_num_args_passed(ASTNode *node) {
    if (!node) return 0;
    else return 1 + count_num_args_passed(node->child1);

}


// adds a declaration to the symbol table or throws an error if necessary
symtab_entry* add_decl(char *lexeme, Type type) {
    if (chk_decl_flag) {
        symtab_entry *entry = symtab_lookup(lexeme);

        if (entry && entry->scope == cur_scope) {
            // double declaration, throw error
            throw_error(line_num, col_num, "Double declaration of", type, lexeme);
        } else {
            // create a new entry in the symbol table -- add it to the desired scope
            symtab_entry *new_entry = add_entry(lexeme, type);

            // return a pointer to the new entry
            return new_entry;
        }
    } return NULL;
}

// create a new symtab entry, add it to the desired scope, and return it
symtab_entry* add_entry(char *lexeme, Type type) {
    symtab_entry* new_entry = (symtab_entry *)calloc(1, sizeof(symtab_entry));

    new_entry->lexeme = lexeme;
    new_entry->type = type;
    // num_args should be 0
    // num_args_passed should be 0
    new_entry->scope = cur_scope;
    // next should be NULL

    // add entry to the desired scope
    symtab_entry *prev, *cur;
    prev = NULL;
    cur = symtab_hds[cur_scope];
    while (cur) {
        prev = cur;
        cur = cur->next;
    }

    if (prev) {
        prev->next = new_entry;
    } else {
        symtab_hds[cur_scope] = new_entry;
    }

    return new_entry;
}

// return most deeply nested instance of lexeme
symtab_entry *symtab_lookup(char *lexeme) {
    symtab_entry *to_return = scope_lookup(lexeme, LOCAL);
    if (!to_return) to_return = scope_lookup(lexeme, GLOBAL);
    return to_return;
}

// perform a lookup for a particular scope in the symbol table
symtab_entry* scope_lookup(char *lexeme, Scope scope) {
    symtab_entry *cur = symtab_hds[scope];
    if (!cur) return NULL;
    while (cur) {
        if (strcmp(lexeme, cur->lexeme) == 0) return cur;
        cur = cur->next;
    } return NULL;
}

// print an err msg to stderr and exit the program
void throw_error(int line_num, int col_num, char *err_msg, Type type, char *lexeme) {
    fprintf(stderr,
            "SEMANTIC ERROR IN LINE %d,%d: %s %s [lexeme: %s]\n",
            line_num,
            col_num,
            err_msg,
            types[type],
            lexeme);
    exit(1);
}

void dump_symtab() {
    char *types[2] = {"FUNC", "VAR"};
    char *scopes[2] = {"GLOBAL", "LOCAL"};

    printf("--------- GLOBAL SCOPE ----------\n");

    symtab_entry *cur = symtab_hds[GLOBAL];
    while (cur) {
        printf("-------------\n%s\n%s\n%d\n%s\n------------\n",
                cur->lexeme,
                types[cur->type],
                cur->num_args,
                scopes[cur->scope]);
        cur = cur->next;
    }

    printf("--------- LOCAL SCOPE -----------\n");

    cur = symtab_hds[LOCAL];
    while (cur) {
        printf("-------------\n%s\n%s\nis_param = %d\nnum_args = %d\n%s\n------------\n",
                cur->lexeme,
                types[cur->type],
                cur->is_param,
                cur->num_args,
                scopes[cur->scope]);
        cur = cur->next;
    }
}

