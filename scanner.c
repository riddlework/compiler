/*
 * File: scanner.c
 * Author: Maria Fay Garcia
 * Purpose: Implement a scanner using a table-driven FSA
 */
#include "scanner.h"

// 40 states
// 27 accept states

// globals
       int    line_num = 1;
       int    col_num  = 0;
       char  *lexeme;           // a pointer to the current lexeme
       int    intcon;             // the current int const val, if any
static char   buf[256];         // a temporary buffer

static char  *ptr;              // a pointer for the buffer
static char   cur_ch;           // the current character being read

       int (*table[40][128])(); // the transition table
       int   cur_state;         // the current state
       int   final_states[27];  // the set of final states


/*************** FUNCTION IMPLEMENTATION *****************/

// initialize the transition table
void setup_table() {
    // intiialize set of final states
    for (int i = 0; i < 27; i++) final_states[i] = i+1;

    // initialize table to all zeros (start state)
    for (int i = 0; i < 39; i++) {
        for (int j = 0; j < 128; j++) {
            table[i][j] = undef;
        }
    }

    // state 0 -- the start state
    table[0]['\t'] =    t29;
    table[0]['\n'] =    t29;
    table[0][' ']  =    t29;
    table[0]['!']  =    t11;
    table[0]['&']  =    t20;
    table[0]['(']  = lparen;
    table[0][')']  = rparen;
    table[0]['*']  =    mul;
    table[0]['+']  =    add;
    table[0][',']  =  comma;
    table[0]['-']  =    sub;
    table[0]['/']  =    t10;
    
    for (int i = 48; i < 58; i++) table[0][i] = t4;  // digits

    table[0][';'] = semi;
    table[0]['<'] =  t17;
    table[0]['='] =   t7;
    table[0]['>'] =  t14;
    
    for (int i = 65; i < 91; i++)  table[0][i] = t1; // capital letters
    for (int i = 97; i < 123; i++) table[0][i] = t1; // lowercase letters

    table[0]['{'] = lbrace;
    table[0]['|'] =    t23;
    table[0]['}'] = rbrace;

    // states 1-27 are all accept states, so we don't have to set them

    // state 28 is for recognizing {letter | digit | _}
    for (int i = 0; i < 128; i++)  table[28][i] = t3; // set all non-members
    for (int i = 48; i < 58; i++)  table[28][i] = t2; // set digits
    for (int i = 65; i < 91; i++)  table[28][i] = t2; // set capital letters
    for (int i = 97; i < 123; i++) table[28][i] = t2; // set lowercase letters
    table[28][95] = t2; // set _
    
    // state 29 is for recognizing digits
    for (int i = 0; i < 128; i++) table[29][i] = t6; // set all non-digits
    for (int i = 48; i < 58; i++) table[29][i] = t5; // reset digits
    
    for (int i = 0; i < 128; i++) {
        // state 30 is for recognizing =
        table[30][i]   = t9; // set all non-{=}
        table[30]['='] = t8;

        // state 31 is for recognizing !=
        table[31][i]   = t13; // set all non-{=}
        table[31]['='] = t12;

        // state 32 is for recognizing > and >=
        table[32][i]   = t16; // set all non-=
        table[32]['='] = t15;

        // state 33 is for recognizing < and <=
        table[33][i]   = t19; // set all non-=
        table[33]['='] = t18;

        // state 34 is for recognizing &&
        table[34][i]   = t22; // set all non-&
        table[34]['&'] = t21;

        // state 35 is for recognizing ||
        table[35][i]   = t25; // set all non-|
        table[35]['|'] = t24;

        // state 36 is for recognizing /*
        table[36][i]   = divide; // set all non-*
        table[36]['*'] =    t26;

        // state 37 is for recognizing the middle of comments
        table[37][i]   = t26; // set all non-*
        table[37]['*'] = t27;

        // state 38 is for finishing comments
        table[38][i]   = t26; // set all non-{*,/}
        table[38]['*'] = t27;
        table[38]['/'] = t28;

        // state 39 is for identifying whitespace
        table[39][i]    = t30; // set all non-whitespace
        table[39]['\t'] = t29;
        table[39]['\n'] = t29;
        table[39][' ']  = t29;
    }
}

// read a token from stdin -- return to client
int get_token() {
    cur_state = 0;
    while (1) {
        cur_ch = getchar();
        if (cur_ch == EOF) return EOF;
        cur_state = table[cur_state][cur_ch](); 
        if (is_final_state(cur_state)) return cur_state;

        // increment column number
        col_num++;
    }
}

// determine if lexeme is a keyword (and what kind) or identifier
int keywd_or_id() {
    if      (strcmp(buf,    "int") == 0) return    kwINT;
    else if (strcmp(buf,     "if") == 0) return     kwIF;
    else if (strcmp(buf,   "else") == 0) return   kwELSE;
    else if (strcmp(buf,  "while") == 0) return  kwWHILE;
    else if (strcmp(buf, "return") == 0) return kwRETURN;
    else                                         return       ID;
}

// return 1 if the current state is an accept state, 0 otherwise
int is_final_state(int cur_state) {
    for (int i = 0; i < 27; i++) {
        if (final_states[i] == cur_state) return 1;
    } return 0;
}


/*************** TRANSITION FUNCTIONS ****************/

// undefined
int undef() { return UNDEF; }

// starting state -> letter
int t1() {
     ptr   = buf;
    *ptr++ = cur_ch;
    return 28;
}

// letter -> letter | digit | _
int t2() {
    *ptr++ = cur_ch;
    return 28;
}

// letter -> non-{letter | digit | _}
int t3() {
    *ptr = '\0';
    ungetc(cur_ch, stdin);
    lexeme = strdup(buf);
    return keywd_or_id();
}

// starting state -> digit
int t4() {
     ptr   = buf;
    *ptr++ = cur_ch;
    return 29;
}

// digit -> digit
int t5() {
    *ptr++ = cur_ch;
    return 29;
}

// digit -> non-digit
int t6() {
    *ptr = '\0';
    ungetc(cur_ch, stdin);
    lexeme = strdup(buf);
    intcon = atoi(buf);
    return INTCON;
}

// starting state -> lparen
int lparen() {
    lexeme = strdup("(");
    return LPAREN;
}

// starting state -> lparen
int rparen() {
    lexeme = strdup(")");
    return RPAREN;
}

// starting state -> lbrace
int lbrace() {
    lexeme = strdup("{");
    return LBRACE;
}

// starting state -> rbrace
int rbrace() {
    lexeme = strdup("}");
    return RBRACE;
}

// starting state -> comma
int comma() {
    lexeme = strdup(",");
    return COMMA;
}

// starting state -> semi
int semi() {
    lexeme = strdup(";");
    return SEMI;
}

// starting state -> =
int t7() { return 30; }

// = -> =
int t8() {
    lexeme = strdup("==");
    return opEQ;
}

// = -> non-{=}
int t9() {
    ungetc(cur_ch, stdin);
    lexeme = strdup("=");
    return opASSG;
}

// starting state -> +
int add() {
    lexeme = strdup("+");
    return opADD;
}

// starting state -> -
int sub() {
    lexeme = strdup("-");
    return opSUB;
}

// starting state -> *
int mul() {
    lexeme = strdup("*");
    return opMUL;
}

// starting state -> /
int t10() { return 36; }

// / -> non-{*}
int divide() {
    ungetc(cur_ch, stdin);
    lexeme = strdup("/");
    return opDIV;
}

// starting state -> !
int t11() { return 31; }

// ! -> =
int t12() {
    lexeme = strdup("!=");
    return opNE;
}

// ! -> non-{!}
int t13() {
    ungetc(cur_ch, stdin);
    lexeme = strdup("!");
    return opNOT;
}

// starting state -> >
int t14() { return 32; }

// > -> =
int t15() {
    lexeme = strdup(">=");
    return opGE;
}

// > -> non-{=}
int t16() {
    ungetc(cur_ch, stdin);
    lexeme = strdup(">");
    return opGT;
}

// starting state -> <
int t17() { return 33; }

// < -> =
int t18() {
    lexeme = strdup("<=");
    return opLE;
}

// < -> non-{=}
int t19() {
    ungetc(cur_ch, stdin);
    lexeme = strdup("<");
    return opLT;
}

// starting state -> &
int t20() { return 34; }

// & -> &
int t21() {
    lexeme = strdup("&&");
    return opAND;
}

// & -> not-{&}
int t22() {
    ungetc(cur_ch, stdin);
    return UNDEF;
}

// starting state -> |
int t23() { return 35; }

// starting state -> | int t23() { return 35; } | -> |
int t24() {
    lexeme = strdup("||");
    return opOR;
}

// | -> not-{|}
int t25() {
    ungetc(cur_ch, stdin);
    return UNDEF;
}

// / -> *
// * -> non-{*}
int t26() { return 37; }

// * -> *
int t27() { return 38; }

// * -> /
int t28() { return UNDEF; }

// starting state -> whitespace
// whitespace -> whitespace
int t29() {
    if (cur_ch == '\n') {
        col_num = 0; // reset column count
        line_num++;  // increment line count
    }
    return 39;
}

// whitespace -> non-whitespace
int t30() {
    ungetc(cur_ch, stdin);
    return UNDEF;
}


