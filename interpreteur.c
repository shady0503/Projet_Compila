#include "interpreteur.h"
#include "semantique.h"
#include "generation_pcode.h"
#include "global.h"

// Mémoire globale pour stocker les valeurs
DataValue MEM[TAILLEMEM];
// Tableau pour stocker le type de chaque valeur en mémoire (int, real, etc.)
DataType  MEM_TYPE[TAILLEMEM];
// SP (Stack Pointer) indique le sommet de la pile et est initialisé à -1 (pile vide)
int SP = -1;
// BP (Base Pointer) est le point de base pour les appels de fonctions/procédures, initialisé à 0
int BP = 0;

// PCi est l'indice de l'instruction courante dans le tableau PCODE utilisé par l'interpréteur
static int PCi = 0; 

// Convertit un entier en float
static float toFloat(int i) { return (float)i; }

// INTER_INST traite une instruction P-code donnée par "inst"
static void INTER_INST(INSTRUCTION inst)
{
    DataValue v1, v2;
    DataType t1, t2;
    int adr;

    // Selon le mnémonique de l'instruction, on fait différentes actions
    switch (inst.MNE)
    {
    case LDI:
        // LDI : Pousse une valeur littérale entière sur la pile.
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow LDI");
        MEM[SP].i = inst.SUITE;
        MEM_TYPE[SP] = TYPE_INT;
        PCi++;
        break;

    case LDA:
        // LDA : Pousse une adresse sur la pile.
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow LDA");
        MEM[SP].i = inst.SUITE;
        MEM_TYPE[SP] = TYPE_INT;
        PCi++;
        break;

    case LDV:
        // LDV : Prend l'adresse sur le haut de pile et remplace par la valeur stockée à cette adresse.
        if (SP < 0) Error("Stack underflow LDV");
        adr = MEM[SP].i;
        if (adr < 0 || adr >= TAILLEMEM) Error("Invalid address LDV");
        MEM[SP] = MEM[adr];
        MEM_TYPE[SP] = MEM_TYPE[adr];
        PCi++;
        break;

    case STO:
        // STO : Dépile la valeur et la stocke dans l'adresse donnée par inst.SUITE.
        // Si SUITE vaut -9999, c'est un simple pop.
        if (SP < 0) Error("Stack underflow STO");
        if (inst.SUITE == -9999)
        {
            SP--;
            PCi++;
            break;
        }
        v1 = MEM[SP];
        t1 = MEM_TYPE[SP];
        SP--;
        adr = inst.SUITE;
        if (adr < 0 || adr >= TAILLEMEM) Error("Invalid address STO");
        MEM[adr] = v1;
        MEM_TYPE[adr] = t1;
        PCi++;
        break;

    case LDL:
    {
        // LDL p : Pousse sur la pile la valeur stockée à l'adresse BP + 2 + p
        int p = inst.SUITE;
        int src = BP + 2 + p;
        if (src < 0 || src >= TAILLEMEM) Error("LDL invalid address");
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow LDL");
        MEM[SP] = MEM[src];
        MEM_TYPE[SP] = MEM_TYPE[src];
        PCi++;
    }
    break;

    case STL:
    {
        // STL p : Dépile la valeur et la stocke dans MEM[BP + 2 + p]
        int p = inst.SUITE;
        if (SP < 0) Error("Stack underflow STL");
        v1 = MEM[SP];
        t1 = MEM_TYPE[SP];
        SP--;
        int dest = BP + 2 + p;
        if (dest < 0 || dest >= TAILLEMEM) Error("STL invalid address");
        MEM[dest] = v1;
        MEM_TYPE[dest] = t1;
        PCi++;
    }
    break;

    case STO_IND:
    {
        // STO_IND : Prend la valeur à la position SP-1 et stocke cette valeur à l'adresse indiquée par la valeur au sommet de pile.
        if (SP < 1) Error("Stack underflow STO_IND");
        int addr = MEM[SP].i;
        if (addr < 0 || addr >= TAILLEMEM) Error("Invalid address STO_IND");
        DataValue val = MEM[SP - 1];
        DataType tt = MEM_TYPE[SP - 1];
        SP -= 2;
        MEM[addr] = val;
        MEM_TYPE[addr] = tt;
        PCi++;
    }
    break;

    case ADD:
    case SUB:
    case MUL:
    case DIVI:
    {
        // Opérations arithmétiques :
        // On dépile les deux opérandes, on effectue l'opération, et on pousse le résultat.
        if (SP < 1) Error("Stack underflow OP");
        v2 = MEM[SP];
        t2 = MEM_TYPE[SP];
        SP--;
        v1 = MEM[SP];
        t1 = MEM_TYPE[SP];
        DataValue res;
        DataType rt;
        // Si l'un des opérandes est réel, on effectue l'opération en float.
        if (t1 == TYPE_REAL || t2 == TYPE_REAL)
        {
            float f1 = (t1 == TYPE_REAL) ? v1.f : toFloat(v1.i);
            float f2 = (t2 == TYPE_REAL) ? v2.f : toFloat(v2.i);
            rt = TYPE_REAL;
            switch (inst.MNE)
            {
            case ADD:  res.f = f1 + f2; break;
            case SUB:  res.f = f1 - f2; break;
            case MUL:  res.f = f1 * f2; break;
            case DIVI:
                if (f2 == 0.0f) Error("Division by zero (float)");
                res.f = f1 / f2;
                break;
            default: break;
            }
        }
        else
        {
            // Sinon, on effectue l'opération en entier.
            rt = TYPE_INT;
            int i1 = v1.i, i2 = v2.i;
            switch (inst.MNE)
            {
            case ADD:  res.i = i1 + i2; break;
            case SUB:  res.i = i1 - i2; break;
            case MUL:  res.i = i1 * i2; break;
            case DIVI:
                if (i2 == 0) Error("Division by zero (int)");
                res.i = i1 / i2;
                break;
            default: break;
            }
        }
        // Remplace les opérandes par le résultat.
        MEM[SP] = res;
        MEM_TYPE[SP] = rt;
        PCi++;
    }
    break;

    case EQL:
    case NEQ:
    case GTR:
    case LSS:
    case GEQ:
    case LEQ:
    {
        // Opérations de comparaison :
        // On dépile deux opérandes, on compare, et on pousse le résultat (1 ou 0)
        if (SP < 1) Error("Stack underflow CMP");
        v2 = MEM[SP];
        t2 = MEM_TYPE[SP];
        SP--;
        v1 = MEM[SP];
        t1 = MEM_TYPE[SP];
        int r = 0;
        // Si l'un des types est réel, la comparaison est faite en float
        if (t1 == TYPE_REAL || t2 == TYPE_REAL)
        {
            float f1 = (t1 == TYPE_REAL) ? v1.f : toFloat(v1.i);
            float f2 = (t2 == TYPE_REAL) ? v2.f : toFloat(v2.i);
            switch (inst.MNE)
            {
            case EQL:  r = (f1 == f2); break;
            case NEQ:  r = (f1 != f2); break;
            case GTR:  r = (f1 >  f2); break;
            case LSS:  r = (f1 <  f2); break;
            case GEQ:  r = (f1 >= f2); break;
            case LEQ:  r = (f1 <= f2); break;
            default: break;
            }
        }
        else
        {
            // Comparaison en entier.
            int i1 = v1.i, i2 = v2.i;
            switch (inst.MNE)
            {
            case EQL:  r = (i1 == i2); break;
            case NEQ:  r = (i1 != i2); break;
            case GTR:  r = (i1 >  i2); break;
            case LSS:  r = (i1 <  i2); break;
            case GEQ:  r = (i1 >= i2); break;
            case LEQ:  r = (i1 <= i2); break;
            default: break;
            }
        }
        // Pousse le résultat de la comparaison sur la pile.
        MEM[SP].i = r;
        MEM_TYPE[SP] = TYPE_INT;
        PCi++;
    }
    break;

    case PRN:
        // PRN : Imprime la valeur en haut de la pile.
        if (SP < 0) Error("Stack underflow PRN");
        if (MEM_TYPE[SP] == TYPE_REAL)
            printf("PRN => %f\n", MEM[SP].f);
        else
            printf("PRN => %d\n", MEM[SP].i);
        SP--;
        PCi++;
        break;

    case INN:
    {
        // INN : Lecture d'une valeur (entrée utilisateur) et stockage à l'adresse spécifiée.
        if (SP < 0) Error("Stack underflow INN");
        adr = MEM[SP].i;
        SP--;
        if (adr < 0 || adr >= TAILLEMEM) Error("Invalid address INN");
        if (MEM_TYPE[adr] == TYPE_REAL)
        {
            float valf;
            printf("Enter a real: ");
            if (scanf("%f", &valf) != 1) Error("Bad input real");
            MEM[adr].f = valf;
            MEM_TYPE[adr] = TYPE_REAL;
        }
        else
        {
            int vali;
            printf("Enter an integer: ");
            if (scanf("%d", &vali) != 1) Error("Bad input int");
            MEM[adr].i = vali;
            MEM_TYPE[adr] = TYPE_INT;
        }
        PCi++;
    }
    break;

    case BZE:
        // BZE : Dépile une condition et branche à l'adresse donnée si la condition vaut 0.
        if (SP < 0) Error("Stack underflow BZE");
        {
            int cond = MEM[SP].i;
            SP--;
            if (cond == 0)
                PCi = inst.SUITE;
            else
                PCi++;
        }
        break;

    case BRN:
        // BRN : Branche inconditionnellement à l'adresse donnée.
        PCi = inst.SUITE;
        break;

    case PUSH_PARAMS_COUNT:
    {
        // PUSH_PARAMS_COUNT : Pousse l'argument (nombre de paramètres) sur la pile.
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow on PUSH_PARAMS_COUNT");
        MEM[SP].i = inst.SUITE;
        MEM_TYPE[SP] = TYPE_INT;
        PCi++;
    }
    break;

    case CALL:
    {
        // CALL : Gère l'appel d'une procédure ou fonction.
        // 1) Dépile le nombre de paramètres.
        if (SP < 0) Error("Stack underflow on CALL (paramCount)");
        int nParams = MEM[SP].i;
        SP--;
        // 2) Sauvegarde l'adresse de retour.
        int retAddr = PCi + 1;
        // 3) Sauvegarde l'ancien BP.
        int oldBP = BP;
        // 4) Pousse retAddr sur la pile.
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow CALL retAddr");
        MEM[SP].i = retAddr;
        MEM_TYPE[SP] = TYPE_INT;
        // 5) Pousse l'ancien BP sur la pile.
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow CALL oldBP");
        MEM[SP].i = oldBP;
        MEM_TYPE[SP] = TYPE_INT;
        // 6) Met à jour BP pour pointer sur la nouvelle base (les deux valeurs sauvegardées).
        BP = SP;
        // Les arguments de la fonction sont ensuite copiés à partir de la pile.
        int startArg = BP - 1 - nParams; // Position du premier argument.
        for (int i = 0; i < nParams; i++)
        {
            MEM[BP + 2 + i]      = MEM[startArg + i];
            MEM_TYPE[BP + 2 + i] = MEM_TYPE[startArg + i];
        }
        // Ajuste SP pour pointer sur le dernier emplacement des paramètres.
        SP = BP + 1 + nParams;
        // Passe à l'adresse de la fonction/procédure appelée.
        PCi = inst.SUITE;
    }
    break;

    case RET:
    {
        // RET : Retour d'une procédure ou fonction.
        // On récupère l'adresse de retour et l'ancien BP, on ajuste la pile et on passe à l'instruction suivante.
        if (BP < 1) Error("Invalid BP in RET");
        int retAddr = MEM[BP - 1].i;
        int oldBP   = MEM[BP].i;
        int n       = inst.SUITE; // Nombre de paramètres utilisateur à enlever
        // Sauvegarde la valeur de retour (située en haut de la pile)
        DataValue retVal = MEM[SP];
        DataType retType = MEM_TYPE[SP];
        // Ajuste SP pour sortir des paramètres
        SP = BP - 2 - n;
        if (SP < -1) Error("Stack pointer negative in RET");
        // Pousse la valeur de retour sur la pile
        SP++;
        MEM[SP] = retVal;
        MEM_TYPE[SP] = retType;
        // Restaure BP et passe à l'adresse de retour
        BP = oldBP;
        PCi = retAddr;
    }
    break;

    case LDF:
    {
        // LDF : Pousse un nombre réel (float) sur la pile à partir d'une représentation en bits.
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow LDF");
        float f;
        memcpy(&f, &inst.SUITE, sizeof(float));
        MEM[SP].f = f;
        MEM_TYPE[SP] = TYPE_REAL;
        PCi++;
    }
    break;

    case HLT:
        // HLT : Arrête l'exécution.
        PCi++;
        break;
    }
}

// Boucle principale de l'interpréteur qui exécute les instructions du P-code
void INTER_PCODE()
{
    PCi = 0;
    SP = -1;
    BP = 0;
    // Exécute les instructions tant que PCi est valide et que l'instruction n'est pas HLT
    while (PCi >= 0 && PCi < TAILLECODE && PCODE[PCi].MNE != HLT)
    {
        INTER_INST(PCODE[PCi]);
    }
    // Si l'instruction HLT est atteinte, on la traite également
    if (PCi < TAILLECODE && PCODE[PCi].MNE == HLT)
    {
        INTER_INST(PCODE[PCi]);
    }
    printf("End of execution (HLT).\n");
}
