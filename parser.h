#include "scanner.h"
#include "symtab.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>

// function stubs
int                 parse(                           );
void                 prog(                           );
void         decl_or_func(char         *passed_lexeme);
void             var_decl(                           );
void              id_list(                           );
void         id_list_rest(                           );
void                 type(                           );
ASTNode*          opt_formals(symtab_entry *func_defn    );
ASTNode*              formals(symtab_entry *func_defn    );
ASTNode*         formals_rest(symtab_entry *func_defn    );
void        opt_var_decls(                           );
ASTNode*        opt_stmt_list(                           );
ASTNode*                 stmt(                           );
ASTNode* fn_call_or_assg_stmt(char         *passed_lexeme);
ASTNode*              if_stmt(                           );
ASTNode*            else_stmt(                           );
ASTNode*           while_stmt(                           );
ASTNode*          return_stmt(                           );
ASTNode*           return_bdy(                           );
ASTNode*        opt_expr_list(symtab_entry *func         );
ASTNode*            expr_list(symtab_entry *func         );
ASTNode*       rest_expr_list(symtab_entry *func         );
ASTNode*             bool_exp(                           );
ASTNode*            arith_exp(symtab_entry *func         );
int                relop(                           );
void                match(Token         expected     );

extern void setup_table();

// globals
extern int           intcon;
extern int           line_num;
extern int           chk_decl_flag;
extern int           print_ast_flag;
extern char         *lexeme;
extern symtab_entry *symtab_hds[2];

