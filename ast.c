#include "ast.h"


// return the NodeType of the AST node pointed to by ptr
NodeType ast_node_type(void *ptr) {
    ASTNode* ast_node = (ASTNode *)ptr;
    return ast_node->type;
}

// return a ptr to the name of the function
char *func_def_name(void *ptr) {
    ASTNode* ast_node = (ASTNode *)ptr;
    return ast_node->symtab_entry->lexeme;
}

// retrun the number of formal parameters for the function
int func_def_nargs(void *ptr) {
    ASTNode* ast_node = (ASTNode *)ptr;
    return ast_node->symtab_entry->num_args;
}

// return a ptr to the ast of the body of the function
void *func_def_body(void *ptr) {
    ASTNode* ast_node = (ASTNode *)ptr;
    return ast_node->child1;
}


// return the name of the nth formal parameter of the function definition
char *func_def_argname(void *ptr, int n) {
    // check that n is within the number of args
    int nargs = func_def_nargs(ptr);
    if (0 < n && n <= nargs) {
        ASTNode* ast_node = (ASTNode *)ptr;

        ASTNode* cur = ast_node;
        for (int i = n; i > 0; i--) cur = cur->child0;
        return cur->symtab_entry->lexeme;
    } return NULL;
}


// return the name of the func being called
char *func_call_callee(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->symtab_entry->lexeme;
}


// returns a ptr to the list of arguments of a function call
void *func_call_args(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child0;
    
}


// returns a ptr to the ast of the stmt at the beginning of opt_stmt_list
void *stmt_list_head(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child0;
}


// returns a pointer to the ast of the rest of opt_stmt_list
void *stmt_list_rest(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child1;
}


// returns a ptr to the ast of the fir expr in opt_expr_list
void *expr_list_head(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child0;

}


// returns a ptr to the rest of opt_expr_list
void *expr_list_rest(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child1;
}


// return the name of an identifier
char *expr_id_name(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->symtab_entry->lexeme;
}


// returns value of integer constant
int expr_intconst_val(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->intcon;
}


// for bool expr, returns ptr to first operand
void *expr_operand_1(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child0;
}


// for bool expr, returns ptr to second operand
void *expr_operand_2(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child1;
}


// returns ptr for if cond
void *stmt_if_expr(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child0;
}


// returns ptr for then body
void *stmt_if_then(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child1;
}


// returns ptr for else body
void *stmt_if_else(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child2;
}


// returns ptr to name of identifier on LHS
char *stmt_assg_lhs(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->symtab_entry->lexeme;
}


// returns ptr to expr on RHS
void *stmt_assg_rhs(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child0;
}


// returns ptr to bool expr
void *stmt_while_expr(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child0;
}

// returns ptr to body
void *stmt_while_body(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child1;
}

// returns ptr to body
void *stmt_return_expr(void *ptr) {
    ASTNode* ast_node = (ASTNode *) ptr;
    return ast_node->child0;
}




