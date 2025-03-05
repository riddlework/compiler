#include "scanner.h"
#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>

// function stubs
int                 parse(              );
void                 prog(              );
void         decl_or_func(char *passed_lexeme  );
void             var_decl(              );
void              id_list(              );
void         id_list_rest(              );
void                 type(              );
void          opt_formals(symtab_entry *func_defn);
void              formals(symtab_entry *func_defn);
void         formals_rest(symtab_entry *func_defn);
void        opt_var_decls(              );
void        opt_stmt_list(              );
void                 stmt(              );
void fn_call_or_assg_stmt(char *passed_lexeme  );
void              if_stmt(              );
void            else_stmt(              );
void           while_stmt(              );
void          return_stmt(              );
void           return_bdy(              );
void        opt_expr_list(symtab_entry *func);
void            expr_list(symtab_entry *func);
void       expr_list_rest(symtab_entry *func);
void             bool_exp(              );
void            arith_exp(symtab_entry *func);
void                relop(              );
void                match(Token expected);

extern void setup_table();

// globals
extern int           line_num;
extern int           chk_decl_flag;
extern char         *lexeme;
extern symtab_entry *symtab_hds[2];

