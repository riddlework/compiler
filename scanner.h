/*
 * File: scanner.h
 * Author: Saumya Debray
 * Purpose: Lists tokens and their values for use in the CSC 453 project
 */

#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
 * The enum Token defines integer values for the various tokens.  These
 * are the values returned by the scanner.
 */
typedef enum {
  UNDEF     /* undefined */,
  ID        /* identifier: e.g., x, abc, p_q_12 */,
  INTCON    /* integer constant: e.g., 12345 */,
  LPAREN    /* '(' : Left parenthesis */,
  RPAREN    /* ')' : Right parenthesis */,
  LBRACE    /* '{' : Left curly brace */,
  RBRACE    /* '}' : Right curly brace */,
  COMMA     /* ',' : Comma */,
  SEMI      /*	;  : Semicolon */,
  kwINT     /*	int */,
  kwIF      /*	if */,
  kwELSE    /*	else */,
  kwWHILE   /*	while */,
  kwRETURN  /*	return */,
  opASSG    /*	= : Assignment */,
  opADD     /*	+ : addition */,
  opSUB     /*	– : subtraction */,
  opMUL     /*	* : multiplication */,
  opDIV     /*	/ : division */,
  opEQ      /*	== : Op: equals */,
  opNE      /*	!= : op: not-equals */,
  opGT      /*	>  : Op: greater-than */,
  opGE      /*	>= : Op: greater-or-equal */,
  opLT      /*	<  : Op: less-than */,
  opLE      /*	<= : Op: less-or-equal */,
  opAND     /*	&& : Op: logical-and */,
  opOR      /*	|| : Op: logical-or */,
  opNOT     /* ! : Op: logical-not */
} Token;

/* function prototypes */
extern int get_token(void);

// function stubs
int get_token     (             );
int keywd_or_id   (             );
int is_final_state(int cur_state);

// transition functions
int  undef();
int     t1();
int     t2();
int     t3();
int     t4();
int     t5();
int     t6();
int lparen();
int rparen();
int lbrace();
int rbrace();
int  comma();
int   semi();
int     t7();
int     t8();
int     t9();
int    add();
int    sub();
int    mul();
int    t10();
int divide();
int    t11();
int    t12();
int    t13();
int    t14();
int    t15();
int    t16();
int    t17();
int    t18();
int    t19();
int    t20();
int    t21();
int    t22();
int    t23();
int    t24();
int    t25();
int    t26();
int    t27();
int    t28();
int    t29();
int    t30();

#endif  /* __SCANNER_H__ */
