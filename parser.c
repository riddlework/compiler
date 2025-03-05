/* File: driver.c
 * Author: Maria Fay Garcia
 * Purpose: Implement a parser that checks the syntax for a G0 program
 */


#include "parser.h"

// globals
Token cur_tok;
Scope cur_scope;

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

void decl_or_func(char *passed_lexeme) {
    if (cur_tok == COMMA) {
        // VAR DECL
        
        // SEMANTIC CHECKING

        // add declaration to symbol table
        add_decl(__func__, passed_lexeme, VAR, NULL);

        match(COMMA);
        id_list();
        match(SEMI);
    } else if (cur_tok == LPAREN) {
        // FUNCTION DECLARATION
        
        // SEMANTIC CHECKING

        // add declaration to symbol table
        symtab_entry *new_entry = add_decl(__func__, passed_lexeme, FUNC, NULL);

        match(LPAREN);
        cur_scope = LOCAL;

        opt_formals(new_entry);
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
        // SEMANTIC CHECKING

        // add the declaration to the symbol table
        add_decl(__func__, passed_lexeme, VAR, NULL);

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

    // add declaration to symbol table
    add_decl(__func__, lexeme, VAR, NULL);

    match(ID);
    id_list_rest();
}

void id_list_rest() {
    if (cur_tok == COMMA) {
        match(COMMA);

        // SEMANTIC CHECKING

        // add declaration to symbol table
        add_decl(__func__, lexeme, VAR, NULL);

        match(ID);
        id_list_rest();
    }
}

void type() {
    match(kwINT);
}

// func_defn - symtab pointer for corresponding func decl
void opt_formals(symtab_entry *func_defn) {
    if (cur_tok == kwINT) {
        formals(func_defn);
    }
}

// func_defn - symtab pointer for corresponding func decl
void formals(symtab_entry *func_defn) {
    type();

    // SEMANTIC CHECKING
    add_decl(__func__, lexeme, VAR, func_defn);

    match(ID);
    formals_rest(func_defn);
}

// entry - symtab pointer for corresponding func decl
void formals_rest(symtab_entry *func_defn) {
    if (cur_tok == COMMA) {
        match(COMMA);
        type();

        // SEMANTIC CHECKING

        // add declaration to symbol table
        add_decl(__func__, lexeme, VAR, func_defn);

        match(ID);
        formals_rest(func_defn);
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
        fn_call_or_assg_stmt(lexeme_to_pass);
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

void fn_call_or_assg_stmt(char *passed_lexeme) {
    if (cur_tok == LPAREN) {
        // function call

        // SEMANTIC CHECKING
        symtab_entry *entry = NULL;
        if (chk_decl_flag) {
            entry = symtab_lookup(passed_lexeme);

            if (!entry) {
                // variable does not exist -- throw error
                throw_error("Use before declaration of", __func__, VAR, passed_lexeme);
            } else if (entry->type == VAR) {
                // an int var tried to act as a function call
                throw_error("Callee is not a", __func__, FUNC, passed_lexeme);
            }
        }

        match(LPAREN);
        opt_expr_list(entry);

        // MORE SEMANTIC CHECKING
        
        // check that func is called with correct number of arguments
        if (chk_decl_flag) {
            if (entry->num_args_passed != entry->num_args) {
                throw_error("Incorrect no. of arguments in call for", __func__, FUNC, passed_lexeme);
            } entry->num_args_passed = 0; // reset after check
        }
        

        match(RPAREN);
    } else {
        // variable assignment
        
        // SEMANTIC CHECKING
        if (chk_decl_flag) {
            symtab_entry *entry = symtab_lookup(passed_lexeme);

            if (!entry) {
                // variable does not exist -- throw error
                throw_error("Use before declaration of", __func__, VAR, passed_lexeme);
            } else if (entry->type == FUNC) {
                // trying to assign an int to a function
                throw_error("Assignment LHS is a", __func__, FUNC, passed_lexeme);
            }
        }

        match(opASSG);
        arith_exp(NULL);
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
    arith_exp(NULL);
}

void opt_expr_list(symtab_entry *func) {
    if (cur_tok == RPAREN) return;
    expr_list(func);
}

void expr_list(symtab_entry *func) {
    arith_exp(func);
    expr_list_rest(func);
}

void expr_list_rest(symtab_entry *func) {
    if (cur_tok == COMMA) {
        match(COMMA);
        arith_exp(func);
        expr_list_rest(func);
    }
}

void bool_exp() {
    arith_exp(NULL);
    relop();
    arith_exp(NULL);
}

void arith_exp(symtab_entry *func) {
    if (cur_tok == ID) {

        // SEMANTIC CHECKING
        if (chk_decl_flag) {

            symtab_entry *entry = symtab_lookup(lexeme);

            if (!entry) {
                // variable does not exist -- throw error
                throw_error("Use before declaration of", __func__, VAR, lexeme);
            } else if (entry->type == FUNC) {
                // arithmetic operation on a func -- throw error
                throw_error("Arithmetic operation on a", __func__, FUNC, lexeme);
            } else if (func) {
                // increment the number of arguments passed to the function
                func->num_args_passed++;
            }
        }

        match(ID);
        
    } else {
        // increment number of arguments passed to function
        if (chk_decl_flag && func) func->num_args_passed++;
        match(INTCON);
    }
}

void relop() {
    if (cur_tok == opEQ) {
        match(opEQ);
    } else if (cur_tok == opNE) {
        match(opNE);
    } else if (cur_tok == opLE) {
        match(opLE);
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
        fprintf(stderr, "SYNTAX ERROR IN LINE %d: unexpected token %d [lexeme = %s]\n", line_num, cur_tok, lexeme);
        exit(1);
    }
}



