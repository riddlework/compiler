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
    "add",
    "neg",
    "sub",
    "div",
    "mul",
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
        case FUNC_DEF:
        {

            // recursively generate the code for the function body
            // skip child 0, as it is just declarations
            gen_three_addr_code(root->child1, -1, -1);

            /*
             * dest  - operand f, the function being defined
             * enter - enter instruction
             * leave - leave instruction
             * return - return instruction
             */
            Operand *dest = new_operand(STPTR, (void *)root->symtab_entry);
            Instr *enter = new_instr(OP_ENTER, NULL, NULL, dest);
            Instr *leave = new_instr(OP_LEAVE, NULL, NULL, dest);
            Instr *ret   = new_instr(OP_RETURN, NULL, NULL, dest);


            // concatenate the three address code
            append_instr(root, enter);
            concat_code(root, root->child1);
            append_instr(root, leave);
            append_instr(root, ret);

            // reset the tmp and lbl counters
            tmp_num = 0;
            break;

        }
        case FUNC_CALL:
        {

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

            // retrieve the return value
            symtab_entry *temp = new_temp();
            Operand *dest = new_operand(STPTR, (void *)temp);
            Instr *retrieve = new_instr(OP_GET_RETVAL, NULL, NULL, dest);

            // concatenate the retrieve instruction
            append_instr(root, retrieve);

            // update place to place of ret val
            root->place = temp;
            break;
        }
        case IF:
        {
            /* LABEL TOMFOOLERY */

            // generate trueDst stuff (then)
            int true_lbl_num = new_label();
            Operand *true_dest = new_operand(LABEL, (void *)&true_lbl_num);
            Instr *true_lbl_instr = new_instr(OP_LABEL, NULL, NULL, true_dest);

            // generate falseDst stuff (else)
            int false_lbl_num = new_label();
            Operand *false_dest = new_operand(LABEL, (void *)&false_lbl_num);
            Instr *false_lbl_instr = new_instr(OP_LABEL, NULL, NULL, false_dest);

            // generate after stuff
            int after_lbl_num = new_label();
            Operand *after_dest = new_operand(LABEL, (void *)&after_lbl_num);
            Instr *after_lbl_instr = new_instr(OP_LABEL, NULL, NULL, after_dest);
            Instr *jump_after = new_instr(OP_GOTO, NULL, NULL, after_dest);

            /* CODE GENERATION */

            // for bool expr -- pass true/false labels
            gen_three_addr_code(root->child0, true_lbl_num, false_lbl_num);

            // then block
            gen_three_addr_code(root->child1, -1, -1);

            // else block
            gen_three_addr_code(root->child2, -1, -1);


            /* CONCATENATION */

            // bool expr
            concat_code(root, root->child0);

            // label instr for true (then)
            append_instr(root, true_lbl_instr);

            // then block
            concat_code(root, root->child1);

            // append jump after instr
            append_instr(root, jump_after);

            // label instr for false (else)
            append_instr(root, false_lbl_instr);

            // concat the code for the else body
            concat_code(root, root->child2);

            // append the label instr for after the else block
            append_instr(root, after_lbl_instr);
            break;
        }
        case WHILE:
        {
            /* LABEL TOMFOOLERY */

            // generate top stuff
            int top_lbl_num = new_label();
            Operand *top_dest = new_operand(LABEL, (void *)&top_lbl_num);
            Instr *top_lbl_instr = new_instr(OP_LABEL, NULL, NULL, top_dest);
            Instr *jump_top = new_instr(OP_GOTO, NULL, NULL, top_dest);

            // generate trueDst stuff (body)
            int true_lbl_num = new_label();
            Operand *true_dest = new_operand(LABEL, (void *)&true_lbl_num);
            Instr *true_lbl_instr = new_instr(OP_LABEL, NULL, NULL, true_dest);

            // generate falseDst stuff (after)
            int false_lbl_num = new_label();
            Operand *false_dest = new_operand(LABEL, (void *)&false_lbl_num);
            Instr *false_lbl_instr = new_instr(OP_LABEL, NULL, NULL, false_dest);


            /* CODE GENERATION */

            // bool expr -- pass true/false labels
            gen_three_addr_code(root->child0, true_lbl_num, false_lbl_num);

            // body
            gen_three_addr_code(root->child1, -1, -1);

            
            /* CONCATENATION */

            // top label
            append_instr(root, top_lbl_instr);

            // bool expr
            concat_code(root, root->child0);

            // body label
            append_instr(root, true_lbl_instr);

            // body code
            concat_code(root, root->child1);

            // jump to top
            append_instr(root, jump_top);

            // after label
            append_instr(root, false_lbl_instr);

            break;
        }
        case ASSG:
        {
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
        case RETURN:
        {
            // generate the code for the body of the return statement
            gen_three_addr_code(root->child0, -1, -1);

            // concat the code
            concat_code(root, root->child0);

            // move the place up
            if (root->child0) root->place = root->child0->place;

            // create and append retval instruction
            Operand *src = new_operand(STPTR, (void *)root->place);
            Instr *set_retval_instr = new_instr(OP_SET_RETVAL, src, NULL, NULL);
            append_instr(root, set_retval_instr);

            // create and append leave instruction
            Instr *leave_instr = new_instr(OP_LEAVE, NULL, NULL, NULL);
            append_instr(root, leave_instr);
            
            // create and append return instruction
            Instr *ret_instr = new_instr(OP_RETURN, NULL, NULL, NULL);
            append_instr(root, ret_instr);

            break;
        }
        case STMT_LIST:
        {
            // recursively generate the code for the statement
            gen_three_addr_code(root->child0, -1, -1);

            // recursively generate the code for the rest of the stmt list
            gen_three_addr_code(root->child1, -1, -1);

            // concatenate the three address code together
            concat_code(root, root->child0);
            concat_code(root, root->child1);

            break;
        }
        case EXPR_LIST:
        {
            // recursively generate the code for the arith_exp
            gen_three_addr_code(root->child0, -1, -1);

            // recursively generate the code for the rest of the expr list
            gen_three_addr_code(root->child1, -1, -1);

            // concatenate the three address code together
            concat_code(root, root->child0);
            concat_code(root, root->child1);

            root->place = root->child0->place;
            break;
        }
        case IDENTIFIER:
        {
            // code: NULL
            // place: symtab_entry
            root->place = root->symtab_entry;
            break;

        }
        case INTCONST:
        {
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
        // swap the bool in the three-addr code gen, so translation to mips is simple
        case EQ:
        case NE:
        case LT:
        case LE:
        case GT:
        case GE:
        {
            // create label operands
            Operand *true_dest = new_operand(LABEL, (void *)&trueDst);
            Operand *false_dest = new_operand(LABEL, (void *)&falseDst);

            /* CODE GENERATION */

            // LHS
            gen_three_addr_code(root->child0, -1, -1);

            // RHS
            gen_three_addr_code(root->child1, -1, -1);

            /* CONCATENATION */
            
            // LHS
            concat_code(root, root->child0);

            // RHS
            concat_code(root, root->child1);


            /*
             * src1 - LHS
             * src2 - RHS
             * type - the OpType (based on NodeType)
             */
            Operand *src1 = new_operand(STPTR, (void *)root->child0->place);
            Operand *src2 = new_operand(STPTR, (void *)root->child1->place);
            OpType op_type = get_opposite_type(root->type);


            // generate comparison instr, jump to false if true (opposite)
            Instr *comp_instr = new_instr(op_type, src1, src2, false_dest);

            // jump to true if we get here
            Instr *jump_true = new_instr(OP_GOTO, NULL, NULL, true_dest);

            // append comparison instr
            append_instr(root, comp_instr);

            // append jump instr
            append_instr(root, jump_true);
            break;
        }
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        {
            /* CODE GENERATION */
            gen_three_addr_code(root->child0, -1, -1); // LHS
            gen_three_addr_code(root->child1, -1, -1); // RHS

            /* CONCATENATION */
            concat_code(root, root->child0); // LHS
            concat_code(root, root->child1); // RHS

            // create a new temp which will store the result of the expr
            symtab_entry *temp = new_temp();

            /*
             * src1 - LHS
             * src2 - RHS
             * dest - place of root
             * type - the OpType (based on NodeType)
             */
            Operand *src1 = new_operand(STPTR, (void *)root->child0->place);
            Operand *src2 = new_operand(STPTR, (void *)root->child1->place);
            Operand *dest = new_operand(STPTR, (void *)temp);
            OpType op_type = get_type(root->type);

            Instr *arith_instr = new_instr(op_type, src1, src2, dest);

            append_instr(root, arith_instr);

            // update place
            root->place = temp;
            break;
        }
        case UMINUS:
        {
            /* CODE GENERATION */
            gen_three_addr_code(root->child0, -1, -1);

            /* CONCATENATION */
            concat_code(root, root->child0);
            
            // create a new temp which will store the result of the expr
            symtab_entry *temp = new_temp();

            /* 
             * src  - RHS
             * dest - place of root
             */
            Operand *src  = new_operand(STPTR, (void *)root->child0->place);
            Operand *dest = new_operand(STPTR, (void *)temp);

            Instr *unary_instr = new_instr(OP_UNARY_MINUS, src, NULL, dest);

            append_instr(root, unary_instr);

            // update place
            root->place = temp;
            break;
        }
        case AND:
        case OR:
        {
            /* LABEL TOMFOOLERY */

            // make intermediate label
            int int_lbl_num = new_label();
            Operand *int_lbl_dest = new_operand(LABEL, (void *)&int_lbl_num);
            Instr *jump_lbl_instr = new_instr(OP_LABEL, NULL, NULL, int_lbl_dest);

            /* CODE GENERATION */
            int trueDst_int  = root->type == AND ? int_lbl_num : trueDst;
            int falseDst_int = root->type == AND ? falseDst : int_lbl_num;

            gen_three_addr_code(root->child0, trueDst_int, falseDst_int);
            gen_three_addr_code(root->child1, trueDst, falseDst);


            /* CONCATENATION */
            concat_code(root, root->child0);
            append_instr(root, jump_lbl_instr);
            concat_code(root, root->child1);
            break;
        }
        default:
            break;
    }
}

void gen_param_instr(ASTNode *concat_to, ASTNode *expr_head) {
    if (expr_head) {
        gen_param_instr(concat_to, expr_head->child1);
        symtab_entry *param_stptr = expr_head->child0->place;
        Operand *src = new_operand(STPTR, (void *)param_stptr);
        Instr *param = new_instr(OP_PARAM, src, NULL, NULL);
        append_instr(concat_to, param);
    }
}

OpType get_type(NodeType type) {
    switch (type) {
        case ADD:
            return OP_PLUS;
        case SUB:
            return OP_MINUS;
        case MUL:
            return OP_MUL;
        case DIV:
            return OP_DIV;
        default:
            // dummy value, execution should never get to this point
            printf("you shouldn't be here...\n");
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
            printf("you shouldn't be here...\n");
            return OP_RETURN;
    }
}


// generate a new operand
// TODO: change to cast instead of dereference
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
    } else if (src && src->code_head) {
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
    printf("# ");
    switch (instr->op) {
        case OP_PLUS:
        case OP_MINUS:
        case OP_DIV:
        case OP_MUL:
        {
            char *lhs  = get_val_string(instr->src1);
            char *rhs  = get_val_string(instr->src2);
            char *dest = get_val_string(instr->dest);
            printf("%s := %s %s %s\n", dest, lhs, op_name[instr->op], rhs);
            break;
        }
        case OP_UNARY_MINUS: 
        {
            char *src  = get_val_string(instr->src1);
            char *dest = get_val_string(instr->dest);
            printf("%s := -%s\n", dest, src);
            break;
        }
        case OP_ASSG:
        {
            char *lhs = get_val_string(instr->dest);
            char *rhs = get_val_string(instr->src1);
            printf("%s %s %s\n", lhs, op_name[instr->op], rhs);
            break;
        }
        case OP_GOTO: {
            char *label = get_val_string(instr->dest);
            printf("goto %s\n", label);
            break;
        }
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
        {
            char *lhs = get_val_string(instr->src1);
            char *rhs = get_val_string(instr->src2);
            printf("%s %s %s\n", lhs, op_name[instr->op], rhs);
            break;
        }
        case OP_LABEL:
        {
            char *label = get_val_string(instr->dest);
            printf("label %s\n", label);
            break;
        }
        case OP_ENTER:
        {
            char *func = get_val_string(instr->dest);
            printf("enter %s\n", func);
            break;
        }
        case OP_LEAVE:
        {
            printf("leave ");
            if (instr->dest) {
                char *func = get_val_string(instr->dest);
                printf("%s", func);
            } printf("\n");
            break;
        }
        case OP_PARAM:
        {
            char *id = get_val_string(instr->src1);
            printf("param %s\n", id);
            break;
        }
        case OP_CALL:
        {
            char *func_name = get_val_string(instr->src1);
            char *num_args = get_val_string(instr->src2);
            printf("call %s, %s\n", func_name, num_args);
            break;
        }
        case OP_RETURN:
        {
            printf("return\n");
            break;
        }
        case OP_SET_RETVAL:
        {
            char *src = get_val_string(instr->src1);
            printf("set_retval %s\n", src);
            break;
        }
        case OP_GET_RETVAL:
        {
            char *dest = get_val_string(instr->dest);
            printf("get_retval %s\n", dest);
            break;
        }
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
    // print_three_addr_code(root);

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
        case OP_PLUS:
        case OP_MINUS:
        case OP_DIV:
        case OP_MUL:
        {
            // unpack the operands
            Operand *src1 = instr->src1;
            Operand *src2 = instr->src2;
            Operand *dest = instr->dest;

            // get the loc strings
            char *src1_loc_string = get_loc_string(src1->val.symtab_ptr);
            char *src2_loc_string = get_loc_string(src2->val.symtab_ptr);
            char *dest_loc_string = get_loc_string(dest->val.symtab_ptr);

            printf(
                    "lw $t0, %s\n"
                    "lw $t1, %s\n"
                    "%s $t2, $t0, $t1\n"
                    "sw $t2, %s\n",
                    src1_loc_string,
                    src2_loc_string,
                    op_name[instr->op],
                    dest_loc_string
                  );
            printf("\n");
            break;
        }
        case OP_UNARY_MINUS:
        {
            // unpack the operands
            Operand *src  = instr->src1;
            Operand *dest = instr->dest;

            // get the loc strings
            char *src_loc_string  = get_loc_string(src->val.symtab_ptr);
            char *dest_loc_string = get_loc_string(dest->val.symtab_ptr);

            printf(
                    "lw $t0, %s\n"
                    "neg $t1, $t0\n"
                    "sw $t1, %s\n",
                    src_loc_string,
                    dest_loc_string
                  );
            printf("\n");
            break;
        }
        case OP_ASSG:
        {
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
        }
        case OP_GOTO: {
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
        case OP_GE:
        {
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
        }
        case OP_LABEL:
        {
            Operand *dest = instr->dest;
            int label = dest->val.label;
            printf("L%d:\n", label);
            printf("\n");
            break;
        }
        case OP_ENTER:
        {
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
        }
        case OP_LEAVE:
        {
            // generate the function epilogue
            /*
             * 1. deallocate locals
             * 2. restore return address
             * 3. restore frame pointer
             * 4. restore stack pointer
             * 5. return to caller
             */
            printf(
                    "# EPILOGUE\n"
                    "la $sp, 0($fp)\n"
                    "lw $ra, 0($sp)\n"
                    "lw $fp, 4($sp)\n"
                    "la $sp, 8($sp)\n"
                  );
            printf("\n");
            break;
        }
        case OP_PARAM: {

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
        }
        case OP_CALL:
        {

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
        case OP_RETURN:
        {
            // generate return jump instruction
            printf("jr $ra\n");
            printf("\n");
            break;
        }
        case OP_SET_RETVAL: {
            char *src_loc_string = get_loc_string(instr->src1->val.symtab_ptr);
            printf("lw $v0, %s\n", src_loc_string);
            printf("\n");
            break;
        }
        case OP_GET_RETVAL: {
            // TODO: this might cause bugs
            char *dest_loc_string = get_loc_string(instr->dest->val.symtab_ptr);
            printf("sw $v0, %s\n", dest_loc_string);
            printf("\n");
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
