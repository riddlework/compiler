/*
 * File: three_addr.h
 * Author: Maria Fay Garcia
 * Purpose: To outline type definitions for generating
 *              three address and mips code
 */
#ifndef __THREE_ADDR_H__
#define __THREE_ADDR_H__

#include "symtab.h"

extern Scope cur_scope;
extern symtab_entry *symtab_hds[2];

// enums
typedef enum {
    ICONST,
    STPTR,
    LABEL
} OperandType;

typedef enum {
    OP_PLUS,
    OP_UNARY_MINUS, // 1 src, 1 dest
    OP_MINUS,       // 2 src, 1 dest
    OP_DIV,
    OP_MUL,
    OP_ASSG,
    OP_GOTO,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LE,
    OP_GT,
    OP_GE,
    OP_LABEL,
    OP_ENTER,
    OP_LEAVE,
    OP_PARAM,
    OP_CALL,
    OP_RETURN,
    OP_SET_RETVAL,
    OP_GET_RETVAL
} OpType;

// structs
typedef struct operand {
    OperandType operand_type;
    union {
        int iconst;
        symtab_entry *symtab_ptr;
        int label;
    } val;
} Operand;

typedef struct instr {
    OpType op;
    Operand *src1;
    Operand *src2;
    Operand *dest;
    struct instr *next;
} Instr;

// include ast.h for the type information
#include "ast.h"

// function stubs
void gen_three_addr_code(ASTNode *root, int true_lbl, int false_lbl);
void gen_param_instr(ASTNode *concat_to, ASTNode *expr_head);
OpType get_type(NodeType type);
OpType get_opposite_type(NodeType type);
Operand *new_operand(OperandType operand_type, void *val);
Instr *new_instr(OpType op, Operand *src1, Operand *src2, Operand *dest);
symtab_entry *new_temp();
int new_label();
void concat_code(ASTNode *dest, ASTNode *src);
void append_instr(ASTNode *dest, Instr *instr);
void print_three_addr_code(ASTNode *root);
void print_three_addr_instr(Instr *instr);
char *get_val_string(Operand *op);
void make_symtab_offsets();
void gen_mips_code(ASTNode* root);
void translate_three_addr_code(Instr *instr);
char *get_loc_string(symtab_entry *loc);
int get_num_locals();
void dump_glob_symtab();
void gen_println_and_main();

#endif  /* __THREE_ADDR_H__ */
