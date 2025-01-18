#ifndef SYNTAXIQUE_H
#define SYNTAXIQUE_H

void Program();
void Bloc();
void Insts();
void Inst();
void Cond();
void Exp();
void Term();
void Fact();
void RepeatInst();
void ForInst();
void CaseInst();

// Procedures/fonctions
void ProcFuncPart();
void ProcDecl();
void FuncDecl();
void CallOrAssign();
static int insideAFunction = 0;      // 0 = not in a function, 1 = inside a function
static char currentFunctionName[64]; // name of the function we are currently parsing

#endif
