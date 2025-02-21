/* File: driver.c
 * Author: Maria Fay Garcia
 * Purpose: Implement a parser that checks the syntax for a G0 program
 */

#include "scanner.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

// function stubs
int          parse(              );
void          prog(              );
void  decl_or_func(              );
void      var_decl(              );
void       id_list(              );
void  id_list_rest(              );
void          type(              );
void   opt_formals(              );
void       formals(              );
void  formals_rest(              );
void opt_var_decls(              );
void opt_stmt_list(              );
void          stmt(              );
void       fn_call(              );
void opt_expr_list(              );
void         match(Token expected);

// globals
       Token cur_tok;
extern int   line_num;
extern int   check_decl_flag;
extern char *lexeme;

SymbolTable *glob_symtab_hd;
SymbolTable *loc_symtab_hd;

extern void setup_table();

// TODO: extend scanner to keep track of line number

// function implementations

int parse() {
    // set up transition table for scanner
    setup_table();

    // get the first token
    cur_tok = get_token();

    // start at the start symbol of the grammar
    prog();
    
    match(EOF);
    return 0;
}


void prog() {
    glob_symtab_hd = (SymbolTable *) calloc(1, sizeof(SymbolTable));

    if (cur_tok == kwINT) {
        type();
        match(ID);
        decl_or_func();
        prog();
    }
}

void decl_or_func() {
    if (cur_tok == COMMA) {
        match(COMMA);
        id_list();
        match(SEMI);
    } else if (cur_tok == LPAREN) {
        match(LPAREN);
        opt_formals();
        match(RPAREN);
        match(LBRACE);
        opt_var_decls();
        opt_stmt_list();
        match(RBRACE);
    }
}

void var_decl() {
    type();
    id_list();
    match(SEMI);
}

void id_list() {
    match(ID);
    id_list_rest();
    match(SEMI);
}

void id_list_rest() {
    if (cur_tok == COMMA) {
        match(COMMA);
        id_list();
    } else {
        match(ID);
    }
}

void type() {
    match(kwINT);
}

void opt_formals() {
    if (cur_tok == kwINT) {
        formals();
    }
}

void formals() {
    type();
    match(ID);
    formals_rest();
}

void formals_rest() {
    if (cur_tok == COMMA) {
        match(COMMA);
        formals();
    }
}

void opt_var_decls() { 
    if (cur_tok == kwINT) {
        var_decl();
        opt_var_decls();
    }
}


void opt_stmt_list() {
    if (cur_tok == ID) {
        stmt();
        opt_stmt_list();
    }
}

void stmt() {
    fn_call();
    match(SEMI);
}

void fn_call() {
    match(ID);
    match(LPAREN);
    opt_expr_list();
    match(RPAREN);
}

void opt_expr_list() { }


// check whether the current token matches the expected one
void match(Token expected) {
    printf("expected token: %d, cur token: %d, lexeme: %s\n", expected, cur_tok, lexeme);
    if (cur_tok == expected) {
        cur_tok = get_token();
    } else {
        // syntax error
        fprintf(stderr, "Syntax error on line %d, exit_status = 1\n", line_num);
        exit(1);
    }
}


int main() {
    SymbolTable glob_symtab_hd;
    SymbolTable loc_symtab_hd;
}

