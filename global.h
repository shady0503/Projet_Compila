#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ----------------------
// Compile-time constants
// ----------------------
#define TAILLECODE 500
#define TAILLEMEM  500
#define TAILLEIDFS 200
#define VAR_BASE   200

// ----------------------
// Language data types
// ----------------------
typedef enum {
    TYPE_INT,
    TYPE_REAL,
    TYPE_BOOL,
    TYPE_STRING,
    TYPE_UNDEF
} DataType;

// ----------------------
// A union for numeric values
// ----------------------
typedef union {
    int   i;
    float f;
} DataValue;

// ----------------------
// Global "memory" and "stack" pointers
// ----------------------
extern DataValue MEM[TAILLEMEM];
extern DataType  MEM_TYPE[TAILLEMEM];
extern int       SP;  // stack pointer
extern int       BP;  // base pointer (for calls)

// ----------------------
// Tokens
// ----------------------
typedef enum {
    PROGRAM_TOKEN,
    VAR_TOKEN,
    CONST_TOKEN,
    TYPE_TOKEN,
    IF_TOKEN,
    THEN_TOKEN,
    ELSE_TOKEN,
    WHILE_TOKEN,
    DO_TOKEN,
    REPEAT_TOKEN,
    UNTIL_TOKEN,
    FOR_TOKEN,
    TO_TOKEN,
    DOWNTO_TOKEN,
    CASE_TOKEN,
    OF_TOKEN,
    BEGIN_TOKEN,
    END_TOKEN,
    WRITE_TOKEN,
    READ_TOKEN,
    PROCEDURE_TOKEN,
    FUNCTION_TOKEN,
    AFFECT_TOKEN, // :=
    PV_TOKEN,     // ;
    PT_TOKEN,     // .
    PLUS_TOKEN,   // +
    MOINS_TOKEN,  // -
    MULTI_TOKEN,  // *
    DIV_TOKEN,    // /
    EGAL_TOKEN,   // =
    DIFF_TOKEN,   // <>
    SUP_TOKEN,    // >
    INF_TOKEN,    // <
    SUPEG_TOKEN,  // >=
    INFEG_TOKEN,  // <=
    COLON_TOKEN,  // :
    PRG_TOKEN,    // (
    PRD_TOKEN,    // )
    VIR_TOKEN,    // ,

    // For literal values
    NUM_TOKEN,
    REAL_TOKEN,
    ID_TOKEN,

    // Type keywords
    INT_TOKEN,
    BOOL_TOKEN,
    STRING_TOKEN,
    // REAL_TOKEN is above

    DIEZE_TOKEN,  // # (EOF)
    ERREUR_TOKEN
} TokenType;

// ----------------------
// Current token structure
// ----------------------
typedef struct {
    TokenType cls;
    char      nom[64];
} TSym_Cour;

extern TSym_Cour symCour;  // current token
extern TSym_Cour symPre;   // previous token
extern FILE*     fsource;
extern int       car_cour;
extern int       line_num;

// ----------------------
// P-code instructions
// ----------------------
typedef enum {
    ADD,
    SUB,
    MUL,
    DIVI,
    EQL,
    NEQ,
    GTR,
    LSS,
    GEQ,
    LEQ,
    PRN,
    INN,
    LDI,
    LDA,
    LDV,
    STO,
    BRN,
    BZE,
    HLT,
    CALL,
    RET,
    LDL,
    STL,
    LDF,
    STO_IND,
    PUSH_PARAMS_COUNT
} Mnemoniques;

typedef struct {
    Mnemoniques MNE;
    int         SUITE;
} INSTRUCTION;

// ----------------------
// Global P-code array
// ----------------------
extern INSTRUCTION PCODE[TAILLECODE];
extern int         PC;

// ----------------------
// Error function
// ----------------------
void Error(const char *msg);

#endif
