#include "interpreteur.h"
#include "semantique.h"
#include "generation_pcode.h"
#include "global.h"

DataValue MEM[TAILLEMEM];
DataType  MEM_TYPE[TAILLEMEM];
int SP = -1;
int BP = 0;

static int PCi = 0; // instruction pointer

static float toFloat(int i) { return (float)i; }

static void INTER_INST(INSTRUCTION inst)
{
    DataValue v1, v2;
    DataType t1, t2;
    int adr;

    switch (inst.MNE)
    {
    case LDI:
        // push an integer literal
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow LDI");
        MEM[SP].i = inst.SUITE;
        MEM_TYPE[SP] = TYPE_INT;
        PCi++;
        break;

    case LDA:
        // push an address
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow LDA");
        MEM[SP].i = inst.SUITE;
        MEM_TYPE[SP] = TYPE_INT;
        PCi++;
        break;

    case LDV:
        // top is an address => read from MEM[addr]
        if (SP < 0) Error("Stack underflow LDV");
        adr = MEM[SP].i;
        if (adr < 0 || adr >= TAILLEMEM) Error("Invalid address LDV");
        MEM[SP] = MEM[adr];
        MEM_TYPE[SP] = MEM_TYPE[adr];
        PCi++;
        break;

    case STO:
        // pop => store into instruction's address (or pop if SUITE=-9999)
        if (SP < 0) Error("Stack underflow STO");
        if (inst.SUITE == -9999)
        {
            // pseudo "pop" usage
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
        // LDL p => push MEM[BP + 2 + p]  (the local param slot p)
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
        // STL p => pop => store in MEM[BP + 2 + p]
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
        // top-1 => value, top => address
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
        if (SP < 1) Error("Stack underflow OP");
        v2 = MEM[SP];
        t2 = MEM_TYPE[SP];
        SP--;
        v1 = MEM[SP];
        t1 = MEM_TYPE[SP];
        DataValue res;
        DataType rt;
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
        if (SP < 1) Error("Stack underflow CMP");
        v2 = MEM[SP];
        t2 = MEM_TYPE[SP];
        SP--;
        v1 = MEM[SP];
        t1 = MEM_TYPE[SP];
        int r = 0;
        // compare as float if either real
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
        MEM[SP].i = r;
        MEM_TYPE[SP] = TYPE_INT;
        PCi++;
    }
    break;

    case PRN:
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
        // pop condition => if 0 => branch
        if (SP < 0) Error("Stack underflow BZE");
        {
            int cond = MEM[SP].i;
            SP--;
            if (cond == 0) PCi = inst.SUITE;
            else PCi++;
        }
        break;

    case BRN:
        PCi = inst.SUITE;
        break;

    case PUSH_PARAMS_COUNT:
    {
        // just push inst.SUITE
        SP++;
        if (SP >= TAILLEMEM) Error("Stack overflow on PUSH_PARAMS_COUNT");
        MEM[SP].i = inst.SUITE;
        MEM_TYPE[SP] = TYPE_INT;
        PCi++;
    }
    break;

case CALL:
{
    // 1) pop paramCount
    if (SP < 0) Error("Stack underflow on CALL (paramCount)");
    int nParams = MEM[SP].i;
    SP--;

    // 2) retAddr = PCi + 1
    int retAddr = PCi + 1;

    // 3) oldBP = BP
    int oldBP = BP;

    // 4) push retAddr
    SP++;
    if (SP >= TAILLEMEM) Error("Stack overflow CALL retAddr");
    MEM[SP].i    = retAddr;
    MEM_TYPE[SP] = TYPE_INT;

    // 5) push oldBP
    SP++;
    if (SP >= TAILLEMEM) Error("Stack overflow CALL oldBP");
    MEM[SP].i    = oldBP;
    MEM_TYPE[SP] = TYPE_INT;

    // 6) BP = SP
    BP = SP; 
    // retAddr is at [BP-1], oldBP at [BP].
    // We want param #0..param #(nParams-1) at [BP+2..BP+2+(nParams-1)].

    // The actual arguments are on top of stack just below this region.
    // Let's figure out from where we copy them:
    int startArg = BP - 1 - nParams; // index of the 1st argument
    for (int i = 0; i < nParams; i++)
    {
        MEM[BP + 2 + i]      = MEM[startArg + i];
        MEM_TYPE[BP + 2 + i] = MEM_TYPE[startArg + i];
    }

    // Now we adjust SP to the last param slot
    SP = BP + 1 + nParams;

    // jump
    PCi = inst.SUITE;
}
break;


case RET:
{
    if (BP < 1) Error("Invalid BP in RET");
    int retAddr = MEM[BP - 1].i;
    int oldBP   = MEM[BP].i;
    int n       = inst.SUITE; // # user parameters to remove

    // pop local slots + param slots + oldBP + retAddr
    SP = BP - 2 - n;
    if (SP < -1) Error("Stack pointer negative in RET");

    BP = oldBP;
    PCi = retAddr;
}
break;

    case LDF:
    {
        // push float from bits
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
        PCi++;
        break;
    }
}

// main loop
void INTER_PCODE()
{
    PCi = 0;
    SP = -1;
    BP = 0;
    while (PCi >= 0 && PCi < TAILLECODE && PCODE[PCi].MNE != HLT)
    {
        INTER_INST(PCODE[PCi]);
    }
    if (PCi < TAILLECODE && PCODE[PCi].MNE == HLT)
    {
        INTER_INST(PCODE[PCi]);
    }
    printf("End of execution (HLT).\n");
}
