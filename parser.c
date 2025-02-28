/* File: driver.c
 * Author: Maria Fay Garcia
 * Purpose: Implement a parser that checks the syntax for a G0 program
 */

#include "scanner.h"
#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>

// function stubs
int                 parse(              );
void                 prog(              );
void         decl_or_func(char *lexeme  );
void             var_decl(              );
void              id_list(              );
void         id_list_rest(              );
void                 type(              );
void          opt_formals(              );
void              formals(              );
void         formals_rest(              );
void        opt_var_decls(              );
void        opt_stmt_list(              );
void                 stmt(              );
void fn_call_or_assg_stmt(char *lexeme  );
void              if_stmt(              );
void            else_stmt(              );
void           while_stmt(              );
void          return_stmt(              );
void           return_bdy(              );
void        opt_expr_list(              );
void            expr_list(              );
void       expr_list_rest(              );
void             bool_exp(              );
void            arith_exp(              );
void                relop(              );
void                match(Token expected);

// globals
       Token         cur_tok;
extern int           line_num;
extern int           chk_decl_flag;
       Scope     cur_scope;
extern char         *lexeme;
extern symtab_entry *symtab_hds[2];

extern void setup_table();

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
    cur_scope = GLOBAL;
    if (cur_tok == kwINT) {
        type();
        char * lexeme_to_pass = lexeme;
        match(ID);
        decl_or_func(lexeme_to_pass);
        prog();
    }
}

void decl_or_func(char * lexeme) {
    if (cur_tok == COMMA) {
        // LIST OF VAR DECLS
        
        // SEMANTIC CHECKING
        add_decl(lexeme, VAR, 0, NULL, cur_scope);

        match(COMMA);
        id_list();
        match(SEMI);
    } else if (cur_tok == LPAREN) {
        // FUNCTION DECLARATION
        
        // SEMANTIC CHECKING
        add_decl(lexeme, FUNC, 0, NULL, cur_scope);

        match(LPAREN);
        cur_scope = LOCAL;

        opt_formals();
        match(RPAREN);
        match(LBRACE);
        opt_var_decls();
        opt_stmt_list();
        match(RBRACE);

        cur_scope = GLOBAL;

        // clear local symbol table
        // TODO: free this memory -- clear local symbol table somewhere else?
        symtab_hds[LOCAL] = NULL;
    } else {
        match(SEMI);
    }
}

void var_decl() {
    type();
    id_list();
    match(SEMI);
}

void id_list() {

    // SEMANTIC CHECKING
    add_decl(lexeme, VAR, 0, NULL, cur_scope);

    match(ID);
    id_list_rest();
}

void id_list_rest() {
    if (cur_tok == COMMA) {
        match(COMMA);

        // SEMANTIC CHECKING
        add_decl(lexeme, VAR, 0, NULL, cur_scope);

        match(ID);
        id_list_rest();
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

    // SEMANTIC CHECKING
    add_decl(lexeme, VAR, 0, NULL, cur_scope);

    match(ID);
    formals_rest();
}

void formals_rest() {
    if (cur_tok == COMMA) {
        match(COMMA);
        type();

        // SEMANTIC CHECKING
        add_decl(lexeme, VAR, 0, NULL, cur_scope);

        match(ID);
        formals_rest();
    }
}

void opt_var_decls() { 
    if (cur_tok == kwINT) {
        var_decl();
        opt_var_decls();
    }
}

void opt_stmt_list() {
    // check follow set for this non-terminal
    if (cur_tok == RBRACE) return;
    stmt();
    opt_stmt_list();
}

void stmt() {
    if (cur_tok == ID) {
        // record lexeme for semantic checking
        char * lexeme_to_pass = lexeme;
        match(ID);
        fn_call_or_assg_stmt(lexeme);
        match(SEMI);
    } else if (cur_tok == kwWHILE) {
        while_stmt();
    } else if (cur_tok == kwIF) {
        if_stmt();
    } else if (cur_tok == kwRETURN) {
        return_stmt();
    } else if (cur_tok == LBRACE) {
        match(LBRACE);
        opt_stmt_list();
        match(RBRACE);
    } else {
        match(SEMI);
    }
}

void fn_call_or_assg_stmt(char *lexeme) {
    // TODO: do i need semantic checking in this function?
    if (cur_tok == LPAREN) {
        // function call

        // SEMANTIC CHECKING
        if (chk_decl_flag) {
            symtab_entry *entry = lookup(lexeme);

            if (!entry) {
                // variable does not exist -- throw error
                fprintf(stderr, "SEMANTIC ERROR: LINE %d: Use before declaration of variable [%s]\n", line_num, lexeme);
                exit(1);
            }
        }

        match(LPAREN);
        opt_expr_list();
        match(RPAREN);
    } else {
        // variable assignment
        
        // SEMANTIC CHECKING
        if (chk_decl_flag) {
            symtab_entry *entry = lookup(lexeme);

            if (!entry) {
                // variable does not exist -- throw error
                fprintf(stderr, "SEMANTIC ERROR: LINE %d: Use before declaration of variable [%s]\n", line_num, lexeme);
                exit(1);
            }
        }

        match(opASSG);
        arith_exp();
    }
}


void if_stmt() {
    match(kwIF);
    match(LPAREN);
    bool_exp();
    match(RPAREN);
    stmt();
    else_stmt();
}

void else_stmt() {
    if (cur_tok == kwELSE) {
        match(kwELSE);
        stmt();
    }
}

void while_stmt() {
    match(kwWHILE);
    match(LPAREN);
    bool_exp();
    match(RPAREN);
    stmt();
}

void return_stmt() {
    match(kwRETURN);
    return_bdy();
    match(SEMI);
}

void return_bdy() {
    if (cur_tok == SEMI) return;
    arith_exp();
}

void opt_expr_list() {
    if (cur_tok == RPAREN) return;
    expr_list();
}

void expr_list() {
    arith_exp();
    expr_list_rest();
}

void expr_list_rest() {
    if (cur_tok == COMMA) {
        match(COMMA);
        arith_exp();
        expr_list_rest();
    }
}

void bool_exp() {
    arith_exp();
    relop();
    arith_exp();
}

void arith_exp() {
    if (cur_tok == ID) {

        // SEMANTIC CHECKING
        if (chk_decl_flag) {
            symtab_entry *entry = lookup(lexeme);

            if (!entry) {
                // variable does not exist -- throw error
                fprintf(stderr, "SEMANTIC ERROR: LINE %d: Use before declaration of variable [%s]\n", line_num, lexeme);
                exit(1);
            }
        }

        match(ID);
        
    } else {
        // TODO: do i need semantic checking here?
        match(INTCON);
    }
}

void relop() {
    if (cur_tok == opEQ) {
        match(opEQ);
    } else if (cur_tok == opNE) {
        match(opNE);
    } else if (cur_tok == opLT) {
        match(opLT);
    } else if (cur_tok == opGE) {
        match(opGE);
    } else {
        match(opGT);
    }
}


// check whether the current token matches the expected one
void match(Token expected) {
    /*printf("expected token: %d, cur token: %d, lexeme: %s\n", expected, cur_tok, lexeme);*/
    if (cur_tok == expected) {
        cur_tok = get_token();
    } else {
        // syntax error
        // TODO: pass rule which was being parsed to error message
        fprintf(stderr, "SYNTAX ERROR: LINE %d: unexpected token %d [lexeme = %s]\n", line_num, cur_tok, lexeme);
        exit(1);
    }
}
