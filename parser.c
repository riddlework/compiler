/* File: driver.c
 * Author: Maria Fay Garcia
 * Purpose: Implement a parser that
 *   -- checks the syntax for the g2 programming language
 *   -- builds symbol table and performs semantic checking
 *   -- builds ast
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

    // put println in the symbol table so program doesn't die
    // (must do only when semantic checking is enable to avoid seg fault)
    if (chk_decl_flag) {
        symtab_entry *println = add_decl("println", FUNC);
        println->num_args = 1;
    }

    // start at the start symbol of the grammar
    prog();

    if (gen_code_flag) do_code_gen_things();

    match(EOF);

    return 0;
}

void do_code_gen_things() {
    // dump the global symbol table to mips
    dump_glob_symtab();

    // generate the code for println and main
    gen_println_and_main();
}

void prog() {
    cur_scope = GLOBAL;
    if (cur_tok == kwINT) {
        type();
        char *lexeme_to_pass = lexeme;
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
        add_decl(passed_lexeme, VAR);

        match(COMMA);
        id_list();
        match(SEMI);
    } else if (cur_tok == LPAREN) {
        // FUNCTION DECLARATION

        // initialize and zero out ast node
        ASTNode *func = (ASTNode *) calloc(1, sizeof(ASTNode));
        func->type = FUNC_DEF;
        
        // SEMANTIC CHECKING

        // add declaration to symbol table
        symtab_entry *new_entry = add_decl(passed_lexeme, FUNC);

        // FILL AST NODE
        func->symtab_entry = new_entry;

        match(LPAREN);
        cur_scope = LOCAL;

        func->child0 = opt_formals();
        match(RPAREN);
        match(LBRACE);
        opt_var_decls();
        
        // add the body of the function to the AST
        func->child1 = opt_stmt_list();

        match(RBRACE);

        // perform semantic checking if requested
        if (chk_decl_flag) perform_semantic_checking(func);

        // print the AST if requested
        if (print_ast_flag) print_ast(func);

        // TODO: FREE AST HERE

        // generate the code if requested
        if (gen_code_flag) gen_mips_code(func);

        // pop the scope and clear local symbol table
        // TODO: free this memory -- clear local symbol table somewhere else?
        cur_scope = GLOBAL;
        symtab_hds[LOCAL] = NULL;

    } else {

        // add the declaration to the symbol table
        add_decl(passed_lexeme, VAR);

        match(SEMI);
    }
}

void var_decl() {
    type();
    id_list();
    match(SEMI);
}

void id_list() {

    // add declaration to symbol table
    add_decl(lexeme, VAR);

    match(ID);
    id_list_rest();
}

void id_list_rest() {
    if (cur_tok == COMMA) {
        match(COMMA);

        // add declaration to symbol table
        add_decl(lexeme, VAR);

        match(ID);
        id_list_rest();
    }
}

void type() {
    match(kwINT);
}

ASTNode* opt_formals() {
    if (cur_tok == kwINT) {
        return formals();
    } return NULL;
}

ASTNode* formals() {

    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
    ast_node->type = EXPR_LIST;

    type();

    // add delcaration to symbol table
    if (chk_decl_flag) {
        symtab_entry *entry = add_decl(lexeme, VAR);
        entry->is_param = 1;
        ast_node->symtab_entry = entry;
    }

    match(ID);
    ast_node->child0 = formals_rest();

    return ast_node;
}

ASTNode* formals_rest() {

    if (cur_tok == COMMA) {
        ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
        ast_node->type = EXPR_LIST;

        match(COMMA);
        type();

        // add declaration to symbol table
        if (chk_decl_flag) {
            symtab_entry *entry = add_decl(lexeme, VAR);
            entry->is_param = 1;
            ast_node->symtab_entry = entry;
        }

        match(ID);

        // add the rest of the declarations
        ast_node->child0 = formals_rest();

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
        char *lexeme_to_pass = lexeme;
        match(ID);
        stmt = fn_call_or_assg_stmt(lexeme_to_pass, line_num, col_num);
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

ASTNode* fn_call_or_assg_stmt(char *passed_lexeme, int line_num, int col_num) {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));

    // for semantic checking
    ast_node->line_num = line_num;
    ast_node->col_num  = col_num;
    ast_node->lexeme   = passed_lexeme;
    symtab_entry *entry = symtab_lookup(passed_lexeme);

    if (cur_tok == LPAREN) {
        // FUNCTION CALL
        
        // set type of ast node
        ast_node->type = FUNC_CALL;

        // add symtab_entry to AST
        ast_node->symtab_entry = entry;

        match(LPAREN);

        // fill the ast with the arguments passed to the function
        ast_node->child0 = opt_expr_list();

        match(RPAREN);
    } else {
        // VARIABLE ASSIGNMENT

        // set type of ast node
        ast_node->type = ASSG;

        // TODO: symtab_entry shouldn't be stored in ASTNode for ASSG...,
        // but it's technically correct here since we only have IDs
        
        // set symtab entry field of ast node
        // this is the LHS of the assg stmt
        ast_node->symtab_entry = entry;

        match(opASSG);

        // fill the RHS of the assg stmt
        ast_node->child0 = arith_exp();
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
    return arith_exp();
}

ASTNode* opt_expr_list() {
    if (cur_tok == RPAREN) return NULL;
    return expr_list();
}

ASTNode* expr_list() {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
    ast_node->type = EXPR_LIST;
    
    ast_node->child0 = arith_exp();
    ast_node->child1 = rest_expr_list();
    
    return ast_node;
}

ASTNode* rest_expr_list() {
    if (cur_tok == COMMA) {
        // initialize and zero out ast node
        ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));
        ast_node->type = EXPR_LIST;

        match(COMMA);
        ast_node->child0 = arith_exp();
        ast_node->child1 = rest_expr_list();

        return ast_node;
    } return NULL;
}

ASTNode* bool_exp() {
    ASTNode *left = bool_exp_2();

    while (cur_tok == opOR) {

        match(opOR);

        ASTNode *new_node = (ASTNode *)calloc(1, sizeof(ASTNode));
        new_node->type = OR;
        new_node->child0 = left;
        new_node->child1 = bool_exp_2();

        left = new_node;
    } return left;
}

ASTNode* bool_exp_2() {
    ASTNode *left = bool_exp_arith();

    while (cur_tok == opAND) {

        match(opAND);

        ASTNode *new_node = (ASTNode *)calloc(1, sizeof(ASTNode));
        new_node->type = AND;
        new_node->child0 = left;
        new_node->child1 = bool_exp_arith();

        left = new_node;
    } return left;

}

ASTNode* bool_exp_arith() {
    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));

    ast_node->child0 = arith_exp();
    ast_node->type = relop();
    ast_node->child1 = arith_exp();
    return ast_node;
}

ASTNode *arith_exp() {

    ASTNode *left = arith_exp_2();

    while (cur_tok == opADD || cur_tok == opSUB) {
        // retrieve what type the AST node should be
        int type = cur_tok == opADD ? ADD : SUB;

        match(cur_tok);

        ASTNode *new_node = (ASTNode *)calloc(1, sizeof(ASTNode));
        new_node->type = type;
        new_node->child0 = left;
        new_node->child1 = arith_exp_2();

        left = new_node;

    } return left;

}

ASTNode* arith_exp_2() {

    ASTNode *left = arith_exp_3();

    while (cur_tok == opMUL || cur_tok == opDIV) {
        // retrieve what type the AST node should be
        int type = cur_tok == opMUL ? MUL : DIV;

        match(cur_tok);

        ASTNode *new_node = (ASTNode *)calloc(1, sizeof(ASTNode));
        new_node->type = type;
        new_node->child0 = left;
        new_node->child1 = arith_exp_3();

        left = new_node;

    } return left;

}

ASTNode* arith_exp_3() {

    if (cur_tok == opSUB) {
        match(opSUB);

        ASTNode *ast_node = (ASTNode *)calloc(1, sizeof(ASTNode));

        ast_node->type = UMINUS;
        ast_node->child0 = arith_exp_3();

        return ast_node;

    } else {
        return arith_exp_4();
    }

}

ASTNode* arith_exp_4() {

    ASTNode *to_return;
    if (cur_tok == LPAREN) {
        match(LPAREN);
        to_return = arith_exp();
        match(RPAREN);
    } else {
        to_return = val();
    } return to_return;

}

ASTNode* val() {

    // initialize and zero out ast node
    ASTNode* ast_node = (ASTNode *) calloc(1, sizeof(ASTNode));

    // for semantic checking
    ast_node->line_num = line_num;
    ast_node->col_num  = col_num;
    ast_node->lexeme   = lexeme;

    if (cur_tok == ID) {

        // ID OR FUNC_CALL

        ast_node->symtab_entry = symtab_lookup(lexeme);
        match(ID);

        if (cur_tok == LPAREN) {
            // FUNC CALL
            ast_node->type = FUNC_CALL;

            match(LPAREN);
            ast_node->child0 = opt_expr_list();
            match(RPAREN);
        } else {
            // ID
            ast_node->type = IDENTIFIER;
        }
    } else {

        // INTCON
        ast_node->type = INTCONST;

        // put integer constant in AST node
        ast_node->intcon = intcon;
        match(INTCON);

    } return ast_node;

}

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
    if (cur_tok == expected) {
        cur_tok = get_token();
    } else {
        // syntax error
        fprintf(stderr, "SYNTAX ERROR IN LINE %d: unexpected token %d [lexeme = %s]\n", line_num, cur_tok, lexeme);
        exit(1);
    }
}

