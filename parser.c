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

// TODO: CHANGE THE WAY YOU KEEP TRACK OF NARGS
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

        // initialize and zero out ast node
        ASTNode *ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
        ast_node->type = FUNC_DEF;
        
        // SEMANTIC CHECKING

        // add declaration to symbol table
        symtab_entry *new_entry = add_decl(__func__, passed_lexeme, FUNC, NULL);

        // FILL AST NODE
        ast_node->symtab_entry = new_entry;

        match(LPAREN);
        cur_scope = LOCAL;

        ast_node->child0 = opt_formals(new_entry);
        match(RPAREN);
        match(LBRACE);
        opt_var_decls();
        
        // add the body of the function to the AST
        ast_node->child1 = opt_stmt_list();

        match(RBRACE);

        cur_scope = GLOBAL;

        // print the AST if requested
        if (print_ast_flag) { print_ast(ast_node); }

        // TODO: FREE AST HERE

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
ASTNode* opt_formals(symtab_entry *func_defn) {
    if (cur_tok == kwINT) {
        return formals(func_defn);
    } return NULL;
}

// func_defn - symtab pointer for corresponding func decl
ASTNode* formals(symtab_entry *func_defn) {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
    ast_node->type = EXPR_LIST;

    type();

    // SEMANTIC CHECKING
    symtab_entry *entry = add_decl(__func__, lexeme, VAR, func_defn);

    ast_node->symtab_entry = entry;

    match(ID);
    ast_node->child0 = formals_rest(func_defn);

    return ast_node;
}

// entry - symtab pointer for corresponding func decl
ASTNode* formals_rest(symtab_entry *func_defn) {
    if (cur_tok == COMMA) {
        ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
        ast_node->type = EXPR_LIST;

        match(COMMA);
        type();

        // SEMANTIC CHECKING

        // add declaration to symbol table
        symtab_entry *entry = add_decl(__func__, lexeme, VAR, func_defn);
        ast_node->symtab_entry = entry;

        match(ID);

        // add the rest of the declarations
        ast_node->child0 = formals_rest(func_defn);

        return ast_node;
    } return NULL;
}

void opt_var_decls() { 
    if (cur_tok == kwINT) {
        var_decl();
        opt_var_decls();
    }
}

ASTNode* opt_stmt_list() {
    // check follow set for this non-terminal
    if (cur_tok == RBRACE) return NULL;

    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
    ast_node->type = STMT_LIST;

    ast_node->child0 = stmt();
    ast_node->child1 = opt_stmt_list();
    
    return ast_node;
}

ASTNode* stmt() {
    ASTNode* stmt = NULL;
    if (cur_tok == ID) {
        // record lexeme for semantic checking
        char * lexeme_to_pass = lexeme;
        match(ID);
        stmt = fn_call_or_assg_stmt(lexeme_to_pass);
        match(SEMI);
    } else if (cur_tok == kwWHILE) {
        stmt = while_stmt();
    } else if (cur_tok == kwIF) {
        stmt = if_stmt();
    } else if (cur_tok == kwRETURN) {
        stmt = return_stmt();
    } else if (cur_tok == LBRACE) {
        match(LBRACE);
        stmt = opt_stmt_list();
        match(RBRACE);
    } else {
        match(SEMI);
    }

    return stmt;
}

ASTNode* fn_call_or_assg_stmt(char *passed_lexeme) {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));

    if (cur_tok == LPAREN) {
        // FUNCTION CALL
        
        // set type of ast node
        ast_node->type = FUNC_CALL;

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

        // fill the ast with the arguments passed to the function
        ast_node->child0 = opt_expr_list(entry);

        // MORE SEMANTIC CHECKING
        
        // check that func is called with correct number of arguments
        if (chk_decl_flag) {
            if (entry->num_args_passed != entry->num_args) {
                throw_error("Incorrect no. of arguments in call for", __func__, FUNC, passed_lexeme);
            } entry->num_args_passed = 0; // reset after check
        }

        // set entry of ast node
        ast_node->symtab_entry = entry;

        match(RPAREN);
    } else {
        // VARIABLE ASSIGNMENT

        // set type of ast node
        ast_node->type = ASSG;
        
        // SEMANTIC CHECKING
        symtab_entry* entry;
        if (chk_decl_flag) {
            entry = symtab_lookup(passed_lexeme);

            if (!entry) {
                // variable does not exist -- throw error
                throw_error("Use before declaration of", __func__, VAR, passed_lexeme);
            } else if (entry->type == FUNC) {
                // trying to assign an int to a function
                throw_error("Assignment LHS is a", __func__, FUNC, passed_lexeme);
            }
        }

        // set symtab entry field of ast node
        // this is the LHS of the assg stmt
        ast_node->symtab_entry = entry;

        match(opASSG);

        // fill the RHS of the assg stmt
        ast_node->child0 = arith_exp(NULL);
    }

    return ast_node;
}


ASTNode* if_stmt() {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
    ast_node->type = IF;

    match(kwIF);
    match(LPAREN);
    ast_node->child0 = bool_exp();
    match(RPAREN);
    ast_node->child1 = stmt();
    ast_node->child2 = else_stmt();

    return ast_node;
}

ASTNode* else_stmt() {
    // initialize and zero out ast node
    if (cur_tok == kwELSE) {
        match(kwELSE);
        return stmt();
    } return NULL;
}

ASTNode* while_stmt() {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
    ast_node->type = WHILE;

    match(kwWHILE);
    match(LPAREN);
    ast_node->child0 = bool_exp();
    match(RPAREN);
    ast_node->child1 = stmt();

    return ast_node;
}

ASTNode* return_stmt() {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
    ast_node->type = RETURN;

    match(kwRETURN);
    ast_node->child0 = return_bdy();
    match(SEMI);

    return ast_node;
}

ASTNode* return_bdy() {
    if (cur_tok == SEMI) return NULL;
    return arith_exp(NULL);
}

ASTNode* opt_expr_list(symtab_entry *func) {
    if (cur_tok == RPAREN) return NULL;
    return expr_list(func);
}

ASTNode* expr_list(symtab_entry *func) {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
    ast_node->type = EXPR_LIST;
    
    ast_node->child0 = arith_exp(func);
    ast_node->child1 = rest_expr_list(func);
    
    return ast_node;
}

ASTNode* rest_expr_list(symtab_entry *func) {
    if (cur_tok == COMMA) {
        // initialize and zero out ast node
        ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
        ast_node->type = EXPR_LIST;

        match(COMMA);
        ast_node->child0 = arith_exp(func);
        ast_node->child1 = rest_expr_list(func);

        return ast_node;
    } return NULL;
}

ASTNode* bool_exp() {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
    /*ast_node->type = EXPR_LIST;*/

    ast_node->child0 = arith_exp(NULL);
    ast_node->type = relop();
    ast_node->child1 = arith_exp(NULL);
    return ast_node;
}

ASTNode* arith_exp(symtab_entry *func) {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));

    if (cur_tok == ID) {
        // VAR
        ast_node->type = IDENTIFIER;

        // SEMANTIC CHECKING
        symtab_entry *entry;
        if (chk_decl_flag) {

            entry = symtab_lookup(lexeme);

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

        ast_node->symtab_entry = entry;

        match(ID);
        
    } else {
        // INTCON
        ast_node->type = INTCONST;

        // increment number of arguments passed to function
        if (chk_decl_flag && func) func->num_args_passed++;
        
        // put integer constant in AST node
        ast_node->intcon = intcon;
        match(INTCON);
    }

    return ast_node;
}

// doesn't return ast... returns type maybe?
// relop token different from ast nodetype
int relop() {
    if (cur_tok == opEQ) {
        match(opEQ);
        return EQ;
    } else if (cur_tok == opNE) {
        match(opNE);
        return NE;
    } else if (cur_tok == opLE) {
        match(opLE);
        return LE;
    } else if (cur_tok == opLT) {
        match(opLT);
        return LT;
    } else if (cur_tok == opGE) {
        match(opGE);
        return GE;
    } else {
        match(opGT);
        return GT;
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



