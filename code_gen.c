#include "code_gen.h"

#define BUFSZ 32

// keep track of the current tmp and label,
// so that these numbers do not conflict
int tmp_num = 0;
int lbl_num = 0;

/*
 * an array of strings that gives, for each operand type,
 * a string indicating the corresponding three-address instruction
 */ 
char *op_name[] = {
    "+",
    "-",
    "-",
    "/",
    "*",
    "=",
    "goto",
    "eq",
    "ne",
    "lt",
    "le",
    "gt",
    "ge",
    "label",
    "enter",
    "leave",
    "param",
    "call",
    "return",
    "set_retval",
    "get_retval"
};

// recursively generate the three address code for this function
void gen_three_addr_code(ASTNode *root, int trueDst, int falseDst) {
    if (!root) return;
    switch (root->type) {
        case FUNC_DEF: {

            // recursively generate the code for the function body
            // skip child 0, as it is just declarations
            gen_three_addr_code(root->child1, -1, -1);

            /*
             * dest  - operand f, the function being defined
             * enter - enter instruction
             * leave - leave instruction
             */
            Operand *dest = new_operand(STPTR, (void *)root->symtab_entry);
            Instr *enter = new_instr(OP_ENTER, NULL, NULL, dest);
            Instr *leave = new_instr(OP_LEAVE, NULL, NULL, dest);


            // concatenate the three address code
            append_instr(root, enter);
            concat_code(root, root->child1);
            append_instr(root, leave);

            // reset the tmp and lbl counters
            tmp_num = 0;
            break;

        } case FUNC_CALL: {

            // recursively generate the expr list code (params)
            gen_three_addr_code(root->child0, -1, -1);

            // concat the code
            concat_code(root, root->child0);

            // generate the param instructions, and add them to the code
            gen_param_instr(root, root->child0);

            /*
             * src1 - func begin called
             * src2 - num of args passed to func
             * call - call instruction
             */
            Operand *src1 = new_operand(STPTR, root->symtab_entry);
            Operand *src2 = new_operand(ICONST, (void *)&root->symtab_entry->num_args);
            Instr *call = new_instr(OP_CALL, src1, src2, NULL);

            // concatenate the call code
            append_instr(root, call);

            // TODO: GET_RET_VAL
            // retrieve the return value
            /*symtab_entry *tmp = new_temp();*/
            /*Operand *dest = new_operand(STPTR, (void *)tmp);*/
            /*Instr *retrieve = new_instr(OP_GET_RETVAL, NULL, NULL, dest);*/
            /**/
            /*// concatenate the retrieve instruction*/
            /*append_instr(root, retrieve);*/
            /**/
            break;

        } case IF: {

            // generate label, operand, instr for false label (JUMP TO ELSE BLOCK)
            int false = new_label();
            Operand *false_dest = new_operand(LABEL, (void *)&false);
            Instr *false_label = new_instr(OP_LABEL, NULL, NULL, false_dest);

            // generate label, operand, instr for after label (JUMP AFTER ELSE BLOCK)
            int after = new_label();
            Operand *after_dest = new_operand(LABEL, (void *)&after);
            Instr *after_label = new_instr(OP_LABEL, NULL, NULL, after_dest);

            // generate the code for the bool expr -- pass label
            gen_three_addr_code(root->child0, -1, false);
            

            // concat the code for the bool expr
            concat_code(root, root->child0);

            // generate the code for the then body
            gen_three_addr_code(root->child1, -1, -1);

            // concat the code for the then body
            concat_code(root, root->child1);

            // generate code for jump to after else block
            Instr *jump_after = new_instr(OP_GOTO, NULL, NULL, after_dest);

            // append the jump instr
            append_instr(root, jump_after);

            // append the label instr for the else block (if false)
            append_instr(root, false_label);

            // generate the code for the else body
            gen_three_addr_code(root->child2, -1, -1);

            // concat the code for the else body
            concat_code(root, root->child2);

            // append the label instr for after the else block
            append_instr(root, after_label);

            break;
        }
        case WHILE: {
            // generate label, operand, instr for false label (JUMP AFTER WHILE LOOP)
            int false = new_label();
            Operand *false_dest = new_operand(LABEL, (void *)&false);
            Instr *false_label = new_instr(OP_LABEL, NULL, NULL, false_dest);

            // generate label, operand, instr for loop label (JUMP TO TOP OF WHILE LOOP)
            int loop = new_label();
            Operand *loop_dest = new_operand(LABEL, (void *)&loop);
            Instr *loop_label = new_instr(OP_LABEL, NULL, NULL, loop_dest);

            // append loop label instr
            append_instr(root, loop_label);

            // generate the code for the bool expr -- pass label
            gen_three_addr_code(root->child0, -1, false);

            // concat the code for the bool expr
            concat_code(root, root->child0);

            // generate the code for the body of the while loop
            gen_three_addr_code(root->child1, -1, -1);

            // concat the code for the body
            concat_code(root, root->child1);

            // make jump instr for top of loop
            Instr *goto_loop = new_instr(OP_GOTO, NULL, NULL, loop_dest);
            // append jump instr
            append_instr(root, goto_loop);

            // append label instr for after while loop (if bool_exp fails)
            append_instr(root, false_label);
            
            break;
        }
        case ASSG: {
            // 1. LHS evaluation is trivial because it is always an id
            //      thus code is NULL
            // 2. evaluate RHS
            // should implicitly generate a new tmp variable
            gen_three_addr_code(root->child0, -1, -1);

            // 3. generate instruction
            /*
             * dest - LHS (variable place) 
             * src1 - tmp from above code generation
             *        ...the place of the child should implicitly be where temp is
             * assg - the assignment instruction
             */
            Operand *dest = new_operand(STPTR, root->symtab_entry);
            Operand *src1 = new_operand(STPTR, root->child0->place);
            Instr *assg = new_instr(OP_ASSG, src1, NULL, dest);

            // 4. concatenate the three address code
            concat_code(root, root->child0);
            append_instr(root, assg);
            break;
        }
        case RETURN: {
            // generate the code for the body of the return statement
            gen_three_addr_code(root->child0, -1, -1);

            // concat the code
            concat_code(root, root->child0);

            // move the place up
            if (root->child0) root->place = root->child0->place;

            // TODO: SET RETVAL
            // create a new set_retval instruction
            /*Operand *src = new_operand(STPTR, (void *)root->place);*/
            /*Instr *set_retval = new_instr(OP_SET_RETVAL, src, NULL, NULL);*/
            /**/
            /*// concat the instruction*/
            /*append_instr(root, set_retval);*/
            /**/

            // create return instruction
            Instr *ret_instr = new_instr(OP_RETURN, NULL, NULL, NULL);

            // concat
            append_instr(root, ret_instr);

            break;
        } case STMT_LIST: {
            // recursively generate the code for the statement
            gen_three_addr_code(root->child0, -1, -1);

            // recursively generate the code for the rest of the stmt list
            gen_three_addr_code(root->child1, -1, -1);

            // concatenate the three address code together
            concat_code(root, root->child0);
            concat_code(root, root->child1);

            break;
        } case EXPR_LIST: {

            // recursively generate the code for the arith_exp
            gen_three_addr_code(root->child0, -1, -1);

            // recursively generate the code for the rest of the expr list
            gen_three_addr_code(root->child1, -1, -1);

            // concatenate the three address code together
            concat_code(root, root->child0);
            concat_code(root, root->child1);

            // TODO: MIGHT CAUSE BUGS
            root->place = root->child0->place;
            break;

        } case IDENTIFIER: {
            // code: NULL
            // place: symtab_entry
            root->place = root->symtab_entry;
            break;

        } case INTCONST: {
            // code: tmp = intcon
            // place: tmp's place in symbol table
            
            // get a new tmp var
            // put it in symbol table
            symtab_entry *tmp_stptr = new_temp();

            /*
             * tmp - destination operand
             * intconst - source operand
             * instr - assignment instruction
             */
            Operand *tmp = new_operand(STPTR, (void *)tmp_stptr);
            Operand *intconst = new_operand(ICONST, (void *)&root->intcon);
            Instr *instr = new_instr(OP_ASSG, intconst, NULL, tmp);

            // add code, place to root
            append_instr(root, instr);
            root->place = tmp_stptr;
            break;

        }
        // swap the bool in the three-addr code gen, so translation is straightforward
        case EQ:
        case NE:
        case LT:
        case LE:
        case GT:
        case GE: {

            // generate code for LHS
            gen_three_addr_code(root->child0, -1, -1);

            // concat the code
            concat_code(root, root->child0);
            
            // generate code for RHS
            gen_three_addr_code(root->child1, -1, -1);

            // concat the code
            concat_code(root, root->child1);

            // create label operand
            Operand *false_dest = new_operand(LABEL, (void *)&falseDst);
            /*
             * src1 - LHS
             * src2 - RHS
             * type - the OpType (based on NodeType)
             */
            Operand *src1 = new_operand(STPTR, (void *)root->child0->place);
            Operand *src2 = new_operand(STPTR, (void *)root->child1->place);

            OpType op_type = get_opposite_type(root->type);
            Instr *instr = new_instr(op_type, src1, src2, false_dest);

            // append instruction
            append_instr(root, instr);
            break;
        }
        case ADD: // not yet
        case SUB: // not yet
        case MUL: // not yet
        case DIV: // not yet
        case UMINUS: // not yet
        case AND: // not yet
        case OR: // not yet
        default:
            break;
    }
}

void gen_param_instr(ASTNode *concat_to, ASTNode *expr_head) {
    if (expr_head) {
        gen_param_instr(concat_to, expr_head->child1);
        symtab_entry *param_stptr = expr_head->child1->place;
        Operand *src = new_operand(STPTR, (void *)param_stptr);
        Instr *param = new_instr(OP_PARAM, src, NULL, NULL);
        append_instr(concat_to, param);
    }
}

OpType get_type(NodeType type) {
    switch (type) {
        case EQ:
            return OP_EQ;
        case NE:
            return OP_NE;
        case LT:
            return OP_LT;
        case LE:
            return OP_LE;
        case GE:
            return OP_GE;
        case GT:
            return OP_GT;
        default:
            // dummy value, execution should never get to this point
            return OP_RETURN;
    }
}

OpType get_opposite_type(NodeType type) {
    switch (type) {
        case EQ:
            return OP_NE;
        case NE:
            return OP_EQ;
        case LT:
            return OP_GE;
        case LE:
            return OP_GT;
        case GE:
            return OP_LT;
        case GT:
            return OP_LE;
        default:
            // dummy value, execution should never get to this point
            return OP_RETURN;
    }

}


// generate a new operand
// TODO: CHANGE FROM DEREFERENCE TO JUST CASTING TO INT?
Operand *new_operand(OperandType operand_type, void *val) {
    Operand *newop = (Operand *)calloc(1, sizeof(Operand));
    newop->operand_type = operand_type;
    switch (operand_type) {
        case ICONST:
            newop->val.iconst = *((int *)val);
            break;
        case STPTR:
            newop->val.symtab_ptr = (symtab_entry *)val;
            break;
        case LABEL:
            newop->val.label = *((int *)val);
            break;
    } return newop;
}

// generate a new instruction
Instr *new_instr(OpType op, Operand *src1, Operand *src2, Operand *dest) {
    Instr *new_instr = (Instr *) calloc(1, sizeof(Instr));
    new_instr->op = op;
    new_instr->src1 = src1;
    new_instr->src2 = src2;
    new_instr->dest = dest;
    return new_instr;
}

// generate a new temporary variable
// add it to the symbol table
symtab_entry *new_temp() {
    char *temp_buf = (char *)malloc(BUFSZ);
    sprintf(temp_buf, "tmp%d", tmp_num++);
    return add_entry(temp_buf, VAR);
}

// generate a new label
int new_label() { return lbl_num++; }

// concatenate the three address code from dest with that of src, store into dest
void concat_code(ASTNode *dest, ASTNode *src) {
    // check for valid src ast node
    if (!dest->code_head && src) {
        // no code...
        // just copy head/tail ptrs from src
        dest->code_head = src->code_head;
        dest->code_tail = src->code_tail;
    } else if (src) {
        // code exists...
        // append and set pointers appropriately
        dest->code_tail->next = src->code_head;
        dest->code_tail = src->code_tail;
    }
}

// append a single instruction to the list of instructions for dest
// works for empty list
void append_instr(ASTNode *dest, Instr *instr) {
    if (!dest->code_head) {
        // no code exists...
        // set head/tail ptrs
        dest->code_head = instr;
        dest->code_tail = instr;
    } else {
        // code exists...
        // append and modify tail ptr appropriately
        dest->code_tail->next = instr;
        dest->code_tail = dest->code_tail->next;
    }
}


// print the three address code for the entire function
// as comments
void print_three_addr_code(ASTNode *root) {
    Instr *cur_instr = root->code_head;
    while (cur_instr) {
        print_three_addr_instr(cur_instr);
        cur_instr = cur_instr->next;
    }
}

// print the three address code for a given instruction
// as comments
void print_three_addr_instr(Instr *instr) {
    //print
    printf("# ");
    switch (instr->op) {
        case OP_PLUS: // not yet
        case OP_UNARY_MINUS: // not yet 
        case OP_MINUS: // not yet
        case OP_DIV: // not yet
        case OP_MUL: // not yet 
        case OP_ASSG: {
            char *lhs = get_val_string(instr->dest);
            char *rhs = get_val_string(instr->src1);
            printf("%s %s %s\n", lhs, op_name[instr->op], rhs);
            break;
        } case OP_GOTO: {
            char *label = get_val_string(instr->dest);
            printf("goto %s\n", label);
            break;
        }
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE: {
            char *lhs = get_val_string(instr->src1);
            char *rhs = get_val_string(instr->src2);
            printf("%s %s %s\n", lhs, op_name[instr->op], rhs);
            break;
        } case OP_LABEL: {
            char *label = get_val_string(instr->dest);
            printf("label %s\n", label);
            break;
        } case OP_ENTER: {
            char *func = get_val_string(instr->dest);
            printf("enter %s\n", func);
            break;
        } case OP_LEAVE: {
            char *func = get_val_string(instr->dest);
            printf("leave %s\n", func);
            break;
        } case OP_PARAM: {
            char *id = get_val_string(instr->src1);
            printf("param %s\n", id);
            break;
        } case OP_CALL: {
            char *func_name = get_val_string(instr->src1);
            char *num_args = get_val_string(instr->src2);
            printf("call %s, %s\n", func_name, num_args);
            break;
        } case OP_RETURN: {
            printf("return\n");
            break;
        }
        case OP_SET_RETVAL: // not yet
        case OP_GET_RETVAL: // not yet
            break;
    }
}

char *get_val_string(Operand *op) {
    char *val_buf = (char *)malloc(BUFSZ);
    switch (op->operand_type) {
        case STPTR: {
            symtab_entry *stptr = op->val.symtab_ptr;
            if (stptr->type == FUNC) {
                sprintf(val_buf, "%s", stptr->lexeme);
            } else {
                char *loc_string = get_loc_string(stptr);
                sprintf(val_buf, "%s [%s]", stptr->lexeme, loc_string);
            }  break;
        } case ICONST: {
            sprintf(val_buf, "%d", op->val.iconst);
            break;
        } case LABEL: {
            sprintf(val_buf, "L%d", op->val.label);
        }
    } return val_buf;
}

void make_symtab_offsets() {
    symtab_entry *cur = symtab_hds[LOCAL];
    int local_fp_offset = -4;
    int param_fp_offset = 8;
    while (cur) {
        if (cur->type == VAR) {
            if (cur->is_param) {
                cur->fp_offset = param_fp_offset;
                param_fp_offset += 4;
            } else {
                // local
                cur->fp_offset = local_fp_offset;
                local_fp_offset -= 4;
            } 
        } cur = cur->next;
    }
}

// generate the mips code for the function
// include the three address code as a comment
void gen_mips_code(ASTNode* root) {
    // generate three address code
    gen_three_addr_code(root, -1, -1);

    // make the symbol table offsets
    make_symtab_offsets();

    // print the three address code for the function
    // in comments above the corresponding mips code
    /*print_three_addr_code(root);*/

    // translate each three address intruction to mips
    Instr *cur_instr = root->code_head;
    while (cur_instr) {
        // print each three address instruction in a comment above the corresponding mips code
        print_three_addr_instr(cur_instr);
        translate_three_addr_code(cur_instr);
        cur_instr = cur_instr->next;
    }
}

void translate_three_addr_code(Instr *instr) {
    switch (instr->op) {
        case OP_PLUS: // not yet
        case OP_UNARY_MINUS: // not yet 
        case OP_MINUS: // not yet
        case OP_DIV: // not yet
        case OP_MUL: // not yet 
        case OP_ASSG: {
            // unpack the operands
            /*
             * src - either ICONST or VAR
             * dest - always VAR
             */
            Operand *src = instr->src1;
            Operand *dest = instr->dest;
            

            /*
             * VAR: lw
             * ICONST: li
             */
            switch (src->operand_type) {
                case VAR: {
                    char *src_loc_string = get_loc_string(src->val.symtab_ptr);
                    printf("lw $t0, %s\n", src_loc_string);
                    break;
                } case ICONST:
                    printf("li $t0, %d\n", src->val.iconst);
                    break;
                case LABEL:
                    // should never be assigning to a label
                    break;
            }

            char *dest_loc_string = get_loc_string(dest->val.symtab_ptr);
            printf("sw $t0, %s\n", dest_loc_string);
            printf("\n");
            break;
        } case OP_GOTO: {
            Operand *dest = instr->dest;
            int label = dest->val.label;
            printf("j L%d\n", label);
            printf("\n");
            break;
        }
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:{
            Operand *src1 = instr->src1;
            Operand *src2 = instr->src2;
            Operand *dest = instr->dest;
            char *src1_loc_string = get_loc_string(src1->val.symtab_ptr);
            char *src2_loc_string = get_loc_string(src2->val.symtab_ptr);
            printf(
                    "lw $t0, %s\n"
                    "lw $t1, %s\n"
                    "b%s $t0, $t1, L%d\n", 
                    src1_loc_string,
                    src2_loc_string,
                    op_name[instr->op],
                    dest->val.label
                    );
            printf("\n");
            break;
       } case OP_LABEL: {
            Operand *dest = instr->dest;
            int label = dest->val.label;
            printf("L%d:\n", label);
            printf("\n");
            break;
       } case OP_ENTER: {
            // generate function prologue
            
            // retrieve the name of the function
            char *func = instr->dest->val.symtab_ptr->lexeme;

            // calculate the number of locals
            int num_locals = get_num_locals();
            
            // write assembler directives
            printf(
                    ".data\n"
                    ".align 2\n"
                    ".text\n"
                    "\n"
                  );
            
            /*
             * 1. allocate space for old $fp and $ra
             * 2. save old $fp
             * 3. save return address
             * 4. update $fp to point into current stack frame
             * 5. allocate space for callee's locals and temps (not params)
             */
            printf(
                    "_%s:\n"
                    "# PROLOGUE\n"
                    "la $sp, -8($sp)\n"
                    "sw $fp, 4($sp)\n"
                    "sw $ra, 0($sp)\n"
                    "la $fp, 0($sp)\n"
                    "la $sp, -%d($sp)\n",
                    func,
                    4*num_locals
                    );
            printf("\n");
            break;
        } case OP_LEAVE: {
            printf("j epilogue\n");
            printf("\n");
            break;
        } case OP_PARAM: {

            char *loc_string = get_loc_string(instr->src1->val.symtab_ptr);

            /* 
             * 1. load param into temp
             * 2. decrement the stack pointer
             * 3. store param onto the stack
             */
            printf(
                    "lw $t0, %s\n"
                    "la $sp, -4($sp)\n"
                    "sw $t0, 0($sp)\n",
                    loc_string
                   );
            printf("\n");
            break;
        } case OP_CALL: {

            // retrieve name of function
            char *func = get_val_string(instr->src1);
            
            // retrieve number of arguments
            int num_args = instr->src2->val.iconst;

            /*
             * 1. jump and link to function
             * 2. increment stack pointer by 4*num_args
             */
            printf(
                    "jal _%s\n"
                    "la $sp, %d($sp)\n",
                    func,
                    4*num_args
                  );
            printf("\n");
            break;
        }
        case OP_RETURN: {
            printf("j epilogue\n");
            printf("\n");
            break;
        } case OP_SET_RETVAL: {
            // TODO:
            /*char *src_loc_string = get_loc_string(instr->src1->val.symtab_ptr);*/
            /*printf("lw %s, $v0\n", src_loc_string);*/
            /*printf("\n");*/
            break;
        } case OP_GET_RETVAL: {
            // TODO:
            /*char *dest_loc_string = get_loc_string(instr->dest->val.symtab_ptr);*/
            /*printf("sw $v0, %s\n", dest_loc_string);*/
            /*printf("\n");*/
            break;
        }
    }

}

// calculate the location of the variable
char *get_loc_string(symtab_entry *loc) {
    char *loc_buf = (char *)malloc(BUFSZ);
    switch (loc->scope) {
        case GLOBAL:
            sprintf(loc_buf, "_%s", loc->lexeme);
            break;
        case LOCAL:
            sprintf(loc_buf, "%d($fp)", loc->fp_offset);
            break;
    } return loc_buf;
}

// calculate the number of local variables
// of a given local scope
// exclude parameters
int get_num_locals() {
    int num_locs = 0;
    symtab_entry* cur = symtab_hds[LOCAL];
    while (cur) {
        if (!cur->is_param) num_locs++;
        cur = cur->next;
    } return num_locs;
}

void dump_glob_symtab() {
    // for global variables, allocate space
    // don't do anything for functions
    symtab_entry *cur = symtab_hds[GLOBAL];
    printf(
            "# dumping global symbol table into mips\n"
            ".data\n"
            );

    if (cur) printf(".align 2\n");
    while (cur) {
        if (cur->type == VAR) {
            printf("_%s:  .space 4\n", cur->lexeme);
        } cur = cur->next;
    } printf("\n");
}

void gen_println_and_main() {
    printf(
            "# hard coded println\n"
            ".align 2\n"
           ".data\n"
           "_nl: .asciiz \"\\n\"\n"
           ".align 2\n"
           ".text\n"
           "# print out an integer followed by a newline\n"
           "_println:\n"
           "li $v0, 1\n"
           "lw $a0, 0($sp)\n"
           "syscall\n"
           "li $v0, 4\n"
           "la $a0, _nl\n"
           "syscall\n"
           "jr $ra\n"
           "\n"
           "# hard coded main call\n"
           ".align 2\n"
           ".text\n"
           "main: j _main\n"
           "\n"
           );

}

void gen_epilogue() {
    // generate function epilogue
    /*
     * 1. deallocate locals
     * 2. restore return address
     * 3. restore frame pointer
     * 4. restore stack pointer
     * 5. return to caller
     */
    printf(
            "epilogue:\n"
            "# EPILOGUE\n"
            "la $sp, 0($fp)\n"
            "lw $ra, 0($sp)\n"
            "lw $fp, 4($sp)\n"
            "la $sp, 8($sp)\n"
            "jr $ra\n"
          );
}
