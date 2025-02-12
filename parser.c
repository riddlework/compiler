/* File: driver.c
 * Author: Maria Fay Garcia
 * Purpose: Implement a parser that checks the syntax for a G0 program
 */

#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>

// function stubs
int          parse(              );
void          prog(              );
void     func_defn(              );
void          type(              );
void   opt_formals(              );
void opt_var_decls(              );
void opt_stmt_list(              );
void          stmt(              );
void       fn_call(              );
void opt_expr_list(              );
void         match(Token expected);

// globals
Token cur_tok;
extern int line_num;
extern char * lexeme;

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
    // call func defn in a loop?
    if (cur_tok == kwINT) {
        func_defn();
        prog();
    }
}

void func_defn() {
    type();
    match(ID);
    match(LPAREN);
    opt_formals();
    match(RPAREN);
    match(LBRACE);
    opt_var_decls();
    opt_stmt_list();
    match(RBRACE);
}

void type() {
    match(kwINT);
}

void opt_formals() { }

void opt_var_decls() { }

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

int first();
int follow();

// check whether the current token matches the expected one
void match(Token expected) {
    /*printf("token: %d, lexeme: %s\n", cur_tok, lexeme);*/
    if (cur_tok == expected) {
        cur_tok = get_token();
    } else {
        // syntax error
        fprintf(stderr, "Syntax error on line %d, exit_status = 1\n", line_num);
        exit(1);
    }
}

