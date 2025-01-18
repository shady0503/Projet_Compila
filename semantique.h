#ifndef SEMANTIQUE_H
#define SEMANTIQUE_H

#include "global.h"

// ----------------------
// Entry type (kind)
typedef enum {
    TVAR,
    TCONST,
    TTYPE,
    TPROC,
    TFUNC
} TTypeIDF;

// ----------------------
// Symbol table entry
// ----------------------
typedef struct {
    char     Nom[32];
    TTypeIDF TIDF;      // variable, const, type, procedure, function
    DataType type;      // actual data type (TYPE_INT, TYPE_REAL, etc.)
    int      Adresse;   // address in memory or code
    int      Value;     // integer value for const or # of params for proc/func
    float    FValue;    // float value for const
} T_IDF;

extern T_IDF TAB_IDFS[TAILLEIDFS];
extern int   NBR_IDFS;
extern int   OFFSET;  // next available global memory address for variables

// ----------------------
// Symbol table checks
// ----------------------
int IDexists(const char* nom);
int isVar(const char* nom);
int isConst(const char* nom);
int getAdresse(const char* nom);
int getConstValue(const char* nom);
float getConstFValue(const char* nom);

int isProcedure(const char* nom);
int isFunction(const char* nom);
int getProcFuncIndex(const char* nom);

// ----------------------
// Declarations
// ----------------------
void TypeDecl();
void ConstDecl();
void VarDecl();

// For var declarations or parameter type
DataType parseBaseType();

#endif
