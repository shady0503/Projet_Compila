#include "semantique.h"
#include "analyse_lexical.h"

T_IDF TAB_IDFS[TAILLEIDFS];
int NBR_IDFS = 0;
int OFFSET = VAR_BASE;

int IDexists(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom))
            return 1;
    }
    return 0;
}
int isVar(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TVAR)
            return 1;
    }
    return 0;
}
int isConst(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TCONST)
            return 1;
    }
    return 0;
}
int getAdresse(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TVAR)
        {
            return TAB_IDFS[i].Adresse;
        }
    }
    Error("Variable not found");
    return -1;
}
int getConstValue(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TCONST &&
            TAB_IDFS[i].type == TYPE_INT)
            return TAB_IDFS[i].Value;
    }
    return 0;
}
float getConstFValue(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TCONST &&
            TAB_IDFS[i].type == TYPE_REAL)
            return TAB_IDFS[i].FValue;
    }
    return 0.0f;
}

int isProcedure(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TPROC)
            return 1;
    }
    return 0;
}
int isFunction(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TFUNC)
            return 1;
    }
    return 0;
}


int getProcFuncIndex(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) &&
            (TAB_IDFS[i].TIDF == TPROC || TAB_IDFS[i].TIDF == TFUNC))
            return i;
    }
    Error("Procedure/Function not found");
    return -1;
}

// -----------------------
// Déclarations
// -----------------------
void TypeDecl()
{
    while (symCour.cls == ID_TOKEN)
    {
        char listIDS[10][32];
        int n = 0;
        do
        {
            if (n >= 10)
                Error("Too many IDs in type alias line");
            strcpy(listIDS[n], symCour.nom);
            n++;
            testSym(ID_TOKEN);
            if (symCour.cls == VIR_TOKEN)
                testSym(VIR_TOKEN);
            else
                break;
        } while (symCour.cls == ID_TOKEN);

        testSym(EGAL_TOKEN);

        DataType d = TYPE_UNDEF;
        if (!strcmp(symCour.nom, "integer"))
        {
            d = TYPE_INT;
            testSym(ID_TOKEN);
        }
        else if (!strcmp(symCour.nom, "real"))
        {
            d = TYPE_REAL;
            testSym(ID_TOKEN);
        }
        else if (!strcmp(symCour.nom, "boolean"))
        {
            d = TYPE_BOOL;
            testSym(ID_TOKEN);
        }
        else if (!strcmp(symCour.nom, "string"))
        {
            d = TYPE_STRING;
            testSym(ID_TOKEN);
        }
        else
        {
            Error("Unknown base type in type alias");
        }

        for (int i = 0; i < n; i++)
        {
            if (IDexists(listIDS[i]))
                Error("Alias name already used");
            strcpy(TAB_IDFS[NBR_IDFS].Nom, listIDS[i]);
            TAB_IDFS[NBR_IDFS].TIDF = TTYPE;
            TAB_IDFS[NBR_IDFS].type = d;
            TAB_IDFS[NBR_IDFS].Adresse = -1;
            NBR_IDFS++;
        }
        if (symCour.cls == PV_TOKEN)
            testSym(PV_TOKEN);
        else
            break;
    }
}

void ConstDecl()
{
    while (symCour.cls == ID_TOKEN)
    {
        char nom[32];
        strcpy(nom, symCour.nom);
        testSym(ID_TOKEN);
        testSym(EGAL_TOKEN);

        if (IDexists(nom))
            Error("Const name declared twice");
        strcpy(TAB_IDFS[NBR_IDFS].Nom, nom);
        TAB_IDFS[NBR_IDFS].TIDF = TCONST;
        TAB_IDFS[NBR_IDFS].Adresse = -1;

        if (symCour.cls == NUM_TOKEN)
        {
            TAB_IDFS[NBR_IDFS].Value = atoi(symCour.nom);
            TAB_IDFS[NBR_IDFS].type = TYPE_INT;
            testSym(NUM_TOKEN);
        }
        else if (symCour.cls == REAL_TOKEN)
        {
            TAB_IDFS[NBR_IDFS].FValue = atof(symCour.nom);
            TAB_IDFS[NBR_IDFS].type = TYPE_REAL;
            testSym(REAL_TOKEN);
        }
        else
        {
            Error("Const must be numeric");
        }
        NBR_IDFS++;
        testSym(PV_TOKEN);
    }
}

void VarDecl()
{
    while (symCour.cls == ID_TOKEN)
    {
        char listIDS[10][32];
        int n = 0;
        do
        {
            if (n >= 10)
                Error("Too many IDs in var line");
            strcpy(listIDS[n], symCour.nom);
            n++;
            testSym(ID_TOKEN);
            if (symCour.cls == VIR_TOKEN)
                testSym(VIR_TOKEN);
            else
                break;
        } while (symCour.cls == ID_TOKEN);

        DataType declaredType = TYPE_INT; // par défaut
        if (symCour.cls == COLON_TOKEN)
        {
            testSym(COLON_TOKEN);
            declaredType = parseBaseType();
        }

        testSym(PV_TOKEN);

        for (int i = 0; i < n; i++)
        {
            if (IDexists(listIDS[i]))
                Error("Var name used");
            strcpy(TAB_IDFS[NBR_IDFS].Nom, listIDS[i]);
            TAB_IDFS[NBR_IDFS].TIDF = TVAR;
            TAB_IDFS[NBR_IDFS].type = declaredType;

            TAB_IDFS[NBR_IDFS].Adresse = OFFSET++;
            printf("Declared variable: %s, Type: %d, Address: %d\n", TAB_IDFS[NBR_IDFS].Nom, TAB_IDFS[NBR_IDFS].type, TAB_IDFS[NBR_IDFS].Adresse);
            NBR_IDFS++;
        }
    }
}

DataType parseBaseType()
{
    if (symCour.cls == INT_TOKEN)
    {
        testSym(INT_TOKEN);
        return TYPE_INT;
    }
    else if (symCour.cls == REAL_TOKEN)
    {
        testSym(REAL_TOKEN);
        return TYPE_REAL;
    }
    else if (symCour.cls == BOOL_TOKEN)
    {
        testSym(BOOL_TOKEN);
        return TYPE_BOOL;
    }
    else if (symCour.cls == STRING_TOKEN)
    {
        testSym(STRING_TOKEN);
        return TYPE_STRING;
    }
    else
    {
        Error("Unknown type in var declaration");
        return TYPE_UNDEF;
    }
}
