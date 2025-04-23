#include "scanner.h"
#include "symtab.h"
#include "ast.h"
#include "code_gen.h"
#include <stdio.h>
#include <stdlib.h>

// function stubs
int                      parse(                   );
void        do_code_gen_things(                   );
void                      prog(                   );
void              decl_or_func(char *passed_lexeme);
void                  var_decl(                   );
void                   id_list(                   );
void              id_list_rest(                   );
void                      type(                   );
ASTNode*           opt_formals(                   );
ASTNode*               formals(                   );
ASTNode*          formals_rest(                   );
void             opt_var_decls(                   );
ASTNode*         opt_stmt_list(                   );
ASTNode*                  stmt(                   );
ASTNode*  fn_call_or_assg_stmt(char *passed_lexeme,
                               int        line_num,
                               int         col_num);
ASTNode*               if_stmt(                   );
ASTNode*             else_stmt(                   );
ASTNode*            while_stmt(                   );
ASTNode*           return_stmt(                   );
ASTNode*            return_bdy(                   );
ASTNode*         opt_expr_list(                   );
ASTNode*             expr_list(                   );
ASTNode*        rest_expr_list(                   );
ASTNode*             arith_exp(                   );
ASTNode*          arith_exp_1a(                   );
ASTNode*           arith_exp_2(                   );
ASTNode*           arith_exp_3(                   );
ASTNode*           arith_exp_4(                   );
ASTNode*                   val(                   );
ASTNode*              bool_exp(                   );
ASTNode*           bool_exp_1a(                   );
ASTNode*            bool_exp_2(                   );
ASTNode*           bool_exp_2a(                   );
ASTNode*        bool_exp_arith(                   );
int                      relop(                   );
void                     match(Token      expected);

extern void setup_table();

// globals
extern int                   intcon;
extern int                 line_num;
extern int                  col_num;
extern int            chk_decl_flag;
extern int           print_ast_flag;
extern int            gen_code_flag;
extern char                 *lexeme;
extern symtab_entry  *symtab_hds[2];

