#include "syntaxique.h"
#include "analyse_lexical.h"
#include "semantique.h"
#include "generation_pcode.h"



// Tableau local pour gérer params
typedef struct
{
    char name[32];
    DataType type;
    int index; // which local index it occupies
} LocalParam;

static LocalParam localParams[50];
static int localCount = 0;

static void initLocalParams()
{
    localCount = 0;
}
static int addLocalParam(const char *nom, DataType t)
{
    strcpy(localParams[localCount].name, nom);
    localParams[localCount].type = t;
    localParams[localCount].index = localCount;
    localCount++;
    return localCount - 1;
}
static int findLocalParamIndex(const char *nom)
{
    for (int i = 0; i < localCount; i++)
    {
        if (!strcmp(localParams[i].name, nom))
            return localParams[i].index;
    }
    return -1;
}
static int isResultKeyword(const char *nm)
{
    // if you lowercase all identifiers, "Result" becomes "result"
    // check if inside function + name == "result"
    return (insideAFunction && strcmp(nm, "result") == 0);
}

// Forward decls
static void parseParamList(int indexProcFunc);
static void parseArguments(int indexProcFunc);

// ---------------------------------------------------------------------
void Program()
{
    testSym(PROGRAM_TOKEN);
    testSym(ID_TOKEN);
    testSym(PV_TOKEN);

    // On autorise plusieurs sections const/type/var
    while (symCour.cls == CONST_TOKEN ||
           symCour.cls == TYPE_TOKEN ||
           symCour.cls == VAR_TOKEN)
    {
        switch (symCour.cls)
        {
        case CONST_TOKEN:
            testSym(CONST_TOKEN);
            ConstDecl();
            break;
        case TYPE_TOKEN:
            testSym(TYPE_TOKEN);
            TypeDecl();
            break;
        case VAR_TOKEN:
            testSym(VAR_TOKEN);
            VarDecl();
            break;
        default:
            break;
        }
    }

    // 1) Génère un BRN 0 => saut provisoire au code principal
    Ecrire2(BRN, 0);
    int jumpOverProcsIndex = PC; // retient l’emplacement de ce BRN

    // 2) Écrit le code des procédures/fonctions
    ProcFuncPart();

    // 3) On fixe la cible du BRN pour pointer après le code des proc/func
    int afterProcs = PC + 1;
    PCODE[jumpOverProcsIndex].SUITE = afterProcs;

    // 4) On parse le bloc principal
    Bloc();
    testSym(PT_TOKEN);
    Ecrire1(HLT);
}

// ---------------------------------------------------------------------
void Bloc()
{
    // Possibles const/type/var locaux
    while (symCour.cls == CONST_TOKEN ||
           symCour.cls == TYPE_TOKEN ||
           symCour.cls == VAR_TOKEN)
    {
        switch (symCour.cls)
        {
        case CONST_TOKEN:
            testSym(CONST_TOKEN);
            ConstDecl();
            break;
        case TYPE_TOKEN:
            testSym(TYPE_TOKEN);
            TypeDecl();
            break;
        case VAR_TOKEN:
            testSym(VAR_TOKEN);
            VarDecl();
            break;
        default:
            break;
        }
    }

    testSym(BEGIN_TOKEN);
    Insts();
    testSym(END_TOKEN);
}

// ---------------------------------------------------------------------
void Insts()
{
    Inst();
    while (symCour.cls == PV_TOKEN)
    {
        testSym(PV_TOKEN);
        if (symCour.cls == END_TOKEN || symCour.cls == UNTIL_TOKEN || symCour.cls == ELSE_TOKEN)
        {
            break;
        }
        Inst();
    }
}

// ---------------------------------------------------------------------
void Inst()
{
    switch (symCour.cls)
    {
    case ID_TOKEN:
        CallOrAssign();
        break;

    case IF_TOKEN:
    {
        testSym(IF_TOKEN);
        Cond();
        testSym(THEN_TOKEN);
        int jumpIf = PC + 1;
        Ecrire2(BZE, 0);
        Inst();
        if (symCour.cls == ELSE_TOKEN)
        {
            int jumpElse = PC + 1;
            Ecrire2(BRN, 0);
            PCODE[jumpIf].SUITE = PC + 1;
            testSym(ELSE_TOKEN);
            Inst();
            PCODE[jumpElse].SUITE = PC + 1;
        }
        else
        {
            PCODE[jumpIf].SUITE = PC + 1;
        }
    }
    break;

    case WHILE_TOKEN:
    {
        testSym(WHILE_TOKEN);
        int condAddr = PC + 1;
        Cond();
        testSym(DO_TOKEN);
        int jumpOut = PC + 1;
        Ecrire2(BZE, 0);
        Inst();
        Ecrire2(BRN, condAddr);
        PCODE[jumpOut].SUITE = PC + 1;
    }
    break;

    case REPEAT_TOKEN:
        RepeatInst();
        break;

    case FOR_TOKEN:
        ForInst();
        break;

    case CASE_TOKEN:
        CaseInst();
        break;

    case BEGIN_TOKEN:
        testSym(BEGIN_TOKEN);
        Insts();
        testSym(END_TOKEN);
        break;

    case WRITE_TOKEN:
    {
        testSym(WRITE_TOKEN);
        testSym(PRG_TOKEN);
        do
        {
            Exp();
            Ecrire1(PRN);
            if (symCour.cls == VIR_TOKEN)
                testSym(VIR_TOKEN);
            else
                break;
        } while (1);
        testSym(PRD_TOKEN);
    }
    break;

    case READ_TOKEN:
    {
        testSym(READ_TOKEN);
        testSym(PRG_TOKEN);
        do
        {
            char nm[32];
            strcpy(nm, symCour.nom);
            testSym(ID_TOKEN);
            int ad = getAdresse(nm);
            Ecrire2(LDI, ad);
            Ecrire1(INN);
            if (symCour.cls == VIR_TOKEN)
                testSym(VIR_TOKEN);
            else
                break;
        } while (1);
        testSym(PRD_TOKEN);
    }
    break;

    default:
        Error("Unknown instruction");
    }
}

// ---------------------------------------------------------------------
// Boolean condition
// ---------------------------------------------------------------------
void Cond()
{
    Exp();
    TokenType t = symCour.cls;
    if (t == EGAL_TOKEN || t == DIFF_TOKEN ||
        t == INF_TOKEN || t == INFEG_TOKEN ||
        t == SUP_TOKEN || t == SUPEG_TOKEN)
    {
        testSym(t);
        Exp();
        switch (t)
        {
        case EGAL_TOKEN:
            Ecrire1(EQL);
            break;
        case DIFF_TOKEN:
            Ecrire1(NEQ);
            break;
        case INF_TOKEN:
            Ecrire1(LSS);
            break;
        case INFEG_TOKEN:
            Ecrire1(LEQ);
            break;
        case SUP_TOKEN:
            Ecrire1(GTR);
            break;
        case SUPEG_TOKEN:
            Ecrire1(GEQ);
            break;
        default:
            break;
        }
    }
    else
    {
        Error("Relational operator expected");
    }
}

// ---------------------------------------------------------------------
void Exp()
{
    Term();
    while (symCour.cls == PLUS_TOKEN || symCour.cls == MOINS_TOKEN)
    {
        TokenType t = symCour.cls;
        testSym(t);
        Term();
        if (t == PLUS_TOKEN)
            Ecrire1(ADD);
        else
            Ecrire1(SUB);
    }
}

// ---------------------------------------------------------------------
void Term()
{
    Fact();
    while (symCour.cls == MULTI_TOKEN || symCour.cls == DIV_TOKEN)
    {
        TokenType t = symCour.cls;
        testSym(t);
        Fact();
        if (t == MULTI_TOKEN)
            Ecrire1(MUL);
        else
            Ecrire1(DIVI);
    }
}

// ---------------------------------------------------------------------
// Fact = ID | NUM | REAL | (Exp) | ...
// ---------------------------------------------------------------------
void Fact()
{
    switch (symCour.cls)
    {
   case ID_TOKEN:
    {
        char nm[32];
        strcpy(nm, symCour.nom);
        testSym(ID_TOKEN); // consume the identifier

        if (isFunction(nm))
        {
            // It's a function. Possibly we have '(' next:
            int idxF = getProcFuncIndex(nm);
            if (symCour.cls == PRG_TOKEN) // '('
            {
                testSym(PRG_TOKEN);
                parseArguments(idxF);
                testSym(PRD_TOKEN);
            }
            else
            {
                // zero-arg function call
                Ecrire2(PUSH_PARAMS_COUNT, 0);
            }
            // Generate CALL
            Ecrire2(CALL, TAB_IDFS[idxF].Adresse);
            // function's result is left on top of stack => perfect for expression
        }
        else if (isProcedure(nm))
        {
            // Pascal disallows procedure in expression
            Error("Cannot call a procedure in an expression context");
        }
        else
        {
            // normal variable / constant / local param / etc.
            int localIdx = findLocalParamIndex(nm);
            if (insideAFunction && localIdx == 0 && !strcmp(nm, currentFunctionName))
            {
                // function result variable => local #0
                Ecrire2(LDL, 0);
            }
            else if (localIdx >= 0)
            {
                // param => pass-by-ref => LDL p; LDV
                Ecrire2(LDL, localIdx);
                Ecrire1(LDV);
            }
            else if (isVar(nm))
            {
                int adr = getAdresse(nm);
                Ecrire2(LDA, adr);
                Ecrire1(LDV);
            }
            else if (isConst(nm))
            {
                // load constant
                DataType ctype = TYPE_UNDEF;
                for(int i=0; i<NBR_IDFS; i++){
                    if(!strcmp(TAB_IDFS[i].Nom, nm) && TAB_IDFS[i].TIDF == TCONST){
                        ctype = TAB_IDFS[i].type;
                        break;
                    }
                }
                if (ctype == TYPE_INT)
                {
                    int c = getConstValue(nm);
                    Ecrire2(LDI, c);
                }
                else if (ctype == TYPE_REAL)
                {
                    float c = getConstFValue(nm);
                    int int_repr;
                    memcpy(&int_repr, &c, sizeof(float));
                    Ecrire2(LDF, int_repr);
                }
                else
                {
                    Error("Unsupported constant type in Fact()");
                }
            }
            else
            {
                Error("Unknown identifier in Fact()");
            }
        }
    }
    break;

    case NUM_TOKEN:
    {
        // Integer literal
        int v = atoi(symCour.nom);
        testSym(NUM_TOKEN);
        Ecrire2(LDI, v);
    }
    break;

    case REAL_TOKEN:
    {
        // Real literal
        float vf = atof(symCour.nom);
        testSym(REAL_TOKEN);
        int int_repr;
        memcpy(&int_repr, &vf, sizeof(float));
        Ecrire2(LDF, int_repr);
    }
    break;

    case PRG_TOKEN:
        testSym(PRG_TOKEN);
        Exp();
        testSym(PRD_TOKEN);
        break;

    default:
        Error("Invalid factor");
        break;
    }
}

// ---------------------------------------------------------------------
void RepeatInst()
{
    testSym(REPEAT_TOKEN);
    int start = PC + 1;
    Insts();
    testSym(UNTIL_TOKEN);
    Cond();
    Ecrire2(BZE, start);
}

// ---------------------------------------------------------------------
void ForInst()
{
    testSym(FOR_TOKEN);
    if (symCour.cls != ID_TOKEN)
        Error("Identifier expected after FOR");
    char varFor[32];
    strcpy(varFor, symCour.nom);
    testSym(ID_TOKEN);
    testSym(AFFECT_TOKEN);
    Exp();
    int addrVar = getAdresse(varFor);
    Ecrire2(STO, addrVar);

    int sens = 0;
    if (symCour.cls == TO_TOKEN)
    {
        sens = 0;
        testSym(TO_TOKEN);
    }
    else if (symCour.cls == DOWNTO_TOKEN)
    {
        sens = 1;
        testSym(DOWNTO_TOKEN);
    }
    else
    {
        Error("TO or DOWNTO expected");
    }

    Exp();
    int slotFin = OFFSET++;
    Ecrire2(STO, slotFin);
    testSym(DO_TOKEN);

    int condAddr = PC + 1;
    Ecrire2(LDA, addrVar);
    Ecrire1(LDV);
    Ecrire2(LDA, slotFin);
    Ecrire1(LDV);
    if (sens == 0)
        Ecrire1(LEQ);
    else
        Ecrire1(GEQ);

    int jumpEnd = PC + 1;
    Ecrire2(BZE, 0);

    Inst();

    Ecrire2(LDA, addrVar);
    Ecrire1(LDV);
    Ecrire2(LDI, 1);
    if (sens == 0)
        Ecrire1(ADD);
    else
        Ecrire1(SUB);
    Ecrire2(STO, addrVar);
    Ecrire2(BRN, condAddr);
    PCODE[jumpEnd].SUITE = PC + 1;
}

// ---------------------------------------------------------------------
void CaseInst()
{
    testSym(CASE_TOKEN);
    Exp();
    int tmpSlot = OFFSET++;
    Ecrire2(STO, tmpSlot);
    testSym(OF_TOKEN);

    int jumpEndLabels[50];
    int nEnd = 0;

    while (symCour.cls == NUM_TOKEN)
    {
        int labelVal = atoi(symCour.nom);
        testSym(NUM_TOKEN);
        testSym(COLON_TOKEN);

        Ecrire2(LDA, tmpSlot);
        Ecrire1(LDV);
        Ecrire2(LDI, labelVal);
        Ecrire1(EQL);

        int jumpIfNot = PC + 1;
        Ecrire2(BZE, 0);
        Inst();

        int jumpAfter = PC + 1;
        Ecrire2(BRN, 0);
        jumpEndLabels[nEnd++] = jumpAfter;
        PCODE[jumpIfNot].SUITE = PC + 1;

        if (symCour.cls == PV_TOKEN)
            testSym(PV_TOKEN);
        else
            break;
    }
    if (symCour.cls == ELSE_TOKEN)
    {
        testSym(ELSE_TOKEN);
        Inst();
        int jumpAfterElse = PC + 1;
        Ecrire2(BRN, 0);
        jumpEndLabels[nEnd++] = jumpAfterElse;
        if (symCour.cls == PV_TOKEN)
            testSym(PV_TOKEN);
    }
    testSym(END_TOKEN);
    int endOfCase = PC + 1;
    for (int i = 0; i < nEnd; i++)
    {
        PCODE[jumpEndLabels[i]].SUITE = endOfCase;
    }
}

// ---------------------------------------------------------------------
// ProcFuncPart() = { ProcDecl | FuncDecl }*
// ---------------------------------------------------------------------
void ProcFuncPart()
{
    while (symCour.cls == PROCEDURE_TOKEN || symCour.cls == FUNCTION_TOKEN)
    {
        if (symCour.cls == PROCEDURE_TOKEN)
            ProcDecl();
        else
            FuncDecl();
    }
}

// ---------------------------------------------------------------------
// procedure <id> (paramlist); Bloc; RET <n>
// ---------------------------------------------------------------------
void ProcDecl()
{
    initLocalParams();

    testSym(PROCEDURE_TOKEN);
    char procName[32];
    strcpy(procName, symCour.nom);
    testSym(ID_TOKEN);

    if (IDexists(procName))
        Error("Procedure name already used");
    int idx = NBR_IDFS;
    strcpy(TAB_IDFS[idx].Nom, procName);
    TAB_IDFS[idx].TIDF = TPROC;
    TAB_IDFS[idx].type = TYPE_UNDEF;
    TAB_IDFS[idx].Adresse = -1;
    TAB_IDFS[idx].Value = 0;
    NBR_IDFS++;

    parseParamList(idx);
    testSym(PV_TOKEN);

    // L'adresse de début du code de la procédure
    int startPC = PC + 1;
    TAB_IDFS[idx].Adresse = startPC;

    // load each parameter from call frame => local param slot
    for (int p = 0; p < TAB_IDFS[idx].Value; p++)
    {
        // param #p => local param #p
        Ecrire2(LDL, p);
        Ecrire2(STL, p);
    }

    Bloc();
    testSym(PV_TOKEN);

    // RET <param_count>
    Ecrire2(RET, TAB_IDFS[idx].Value);
}

// ---------------------------------------------------------------------
// function <id> (paramlist) : <type>; Bloc; (push result) RET <n>
// ---------------------------------------------------------------------
void FuncDecl()
{
    initLocalParams();

    testSym(FUNCTION_TOKEN);

    char fnName[32];
    strcpy(fnName, symCour.nom);
    testSym(ID_TOKEN);

    // Create a table entry for the function ...
    int idx = NBR_IDFS;
    strcpy(TAB_IDFS[idx].Nom, fnName);
    TAB_IDFS[idx].TIDF = TFUNC;
    TAB_IDFS[idx].Adresse = -1;
    TAB_IDFS[idx].Value = 0; // temporarily
    TAB_IDFS[idx].type = TYPE_INT; // default, replaced later
    NBR_IDFS++;

    // parseParamList() reads "(...)" and sets TAB_IDFS[idx].Value
    parseParamList(idx);

    // read ': <type>'
    testSym(COLON_TOKEN);
    DataType retType = parseBaseType();
    TAB_IDFS[idx].type = retType;

    testSym(PV_TOKEN);

    // *** NEW/IMPORTANT: Mark that we are in a function ***
    insideAFunction = 1;
    strcpy(currentFunctionName, fnName);

    // The code start (function body) => set address
    int startPC = PC + 1;
    TAB_IDFS[idx].Adresse = startPC;

    // SHIFT user parameters up by 1 => local #1..#N
    // local #0 => the function's result
    int userParamCount = TAB_IDFS[idx].Value;
    for (int p = userParamCount - 1; p >= 0; p--)
    {
        localParams[p+1] = localParams[p];
        localParams[p+1].index = p+1;
    }
    // local #0 => store the function result
    strcpy(localParams[0].name, fnName);
    localParams[0].type = retType;
    localParams[0].index = 0;
    localCount = userParamCount + 1;

    // So the final #locals is #userParams + 1
    TAB_IDFS[idx].Value = userParamCount + 1;

    // Now load user parameters from the call frame => local #1..#N
    // param #(p-1) => local #p
    for (int p = 1; p <= userParamCount; p++)
    {
        Ecrire2(LDL, p-1);
        Ecrire2(STL, p);
    }

    // Parse the function body
    Bloc();

    testSym(PV_TOKEN);

    // *** At the very end, we push local #0 => the function value
    Ecrire2(LDL, 0);

    // RET <num_user_params>
    Ecrire2(RET, userParamCount);

    // *** No longer in function
    insideAFunction = 0;
    currentFunctionName[0] = '\0';
}

// ---------------------------------------------------------------------
// Appel d'ID => "id(...)" ou "id := exp"
// ---------------------------------------------------------------------
void CallOrAssign()
{
    char name[32];
    strcpy(name, symCour.nom);
    testSym(ID_TOKEN); // consume the identifier

    // Check if known procedure or function
    int isProc = isProcedure(name);
    int isFunc = isFunction(name);

    if (isProc || isFunc)
    {
        // => procedure or function call
        int idxPF = getProcFuncIndex(name);

        // parse optional argument list
        if (symCour.cls == PRG_TOKEN) {
            testSym(PRG_TOKEN);
            parseArguments(idxPF);
            testSym(PRD_TOKEN);
        } else {
            // zero arguments
            Ecrire2(PUSH_PARAMS_COUNT, 0);
        }
        // generate the CALL
        Ecrire2(CALL, TAB_IDFS[idxPF].Adresse);

        if (isFunc) {
            // If a function is called in a statement context, discard result
            Ecrire2(STO, -9999);  // "pop"
        }
    }
    else
    {
        // => assignment "ID := <Expr>"
        testSym(AFFECT_TOKEN);
        Exp();  // parse the right-side expression => value on top of stack

        // *** If inside a function & name == currentFunctionName => local #0
        if (insideAFunction && (strcmp(name, currentFunctionName) == 0)) {
            Ecrire2(STL, 0);  // store in local #0 => function's own name
        } else {
            // normal variable or parameter
            int localIdx = findLocalParamIndex(name);
            if (localIdx >= 0) {
                // param => pass-by-ref => LDL p; STO_IND
                Ecrire2(LDL, localIdx);
                Ecrire1(STO_IND);
            } else {
                // global var => STO <address>
                int adr = getAdresse(name);
                Ecrire2(STO, adr);
            }
        }
    }
}

// ---------------------------------------------------------------------
// parseParamList => fill localParams[] with user param names
// ---------------------------------------------------------------------
static void parseParamList(int indexProcFunc)
{
    int total = 0;
    if (symCour.cls == PRG_TOKEN)
    {
        testSym(PRG_TOKEN);
        while (symCour.cls == ID_TOKEN)
        {
            char groupIDS[10][32];
            int gCount = 0;
            // parse "x,y,..."
            while (symCour.cls == ID_TOKEN)
            {
                strcpy(groupIDS[gCount], symCour.nom);
                gCount++;
                testSym(ID_TOKEN);
                if (symCour.cls == VIR_TOKEN)
                    testSym(VIR_TOKEN);
                else
                    break;
            }
            testSym(COLON_TOKEN);
            DataType ptype = parseBaseType();

            // record each ID
            for (int i = 0; i < gCount; i++)
            {
                addLocalParam(groupIDS[i], ptype);
                total++;
            }

            if (symCour.cls == PV_TOKEN)
                testSym(PV_TOKEN);
            else
                break;
        }
        testSym(PRD_TOKEN);
    }
    TAB_IDFS[indexProcFunc].Value = total; // store # user params for now
}

// ---------------------------------------------------------------------
// parseArguments => parse the actual args "expr, expr, ..."
// for pass-by-reference => if user writes ID => we push address
// ---------------------------------------------------------------------
static void parseArguments(int indexProcFunc)
{
    // Number of declared parameters
    int nbParams = TAB_IDFS[indexProcFunc].Value;
    // If it's a function, subtract 1 for the function-result slot
    if (TAB_IDFS[indexProcFunc].TIDF == TFUNC) {
        nbParams--;
    }

    int count = 0;

    // If the next token is not ')' then we have some arguments
    if (symCour.cls != PRD_TOKEN)
    {
        while (1)
        {
            // If the argument is an identifier => push address (pass-by-ref)
            if (symCour.cls == ID_TOKEN)
            {
                int addr = getAdresse(symCour.nom);
                // LDA <address>
                Ecrire2(LDA, addr);
                testSym(ID_TOKEN);
            }
            else
            {
                // Otherwise => parse as expression => push value (pass-by-value)
                Exp();
            }
            count++;

            // If comma, consume and parse the next argument
            if (symCour.cls == VIR_TOKEN) testSym(VIR_TOKEN);
            else break;
        }
    }

    // Check argument count vs. declared parameters
    if (count != nbParams)
    {
        char buf[128];
        sprintf(buf, "Incorrect number of parameters. Expected %d, got %d",
                nbParams, count);
        Error(buf);
    }

    // Finally push the param-count for the CALL
    Ecrire2(PUSH_PARAMS_COUNT, count);
}
