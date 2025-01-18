#include "analyse_lexical.h"

TSym_Cour symCour;
TSym_Cour symPre;
FILE*     fsource = NULL;
int       car_cour = 0;
int       line_num = 1;

void Error(const char* msg)
{
    fprintf(stderr, "Error line %d: %s (last token: '%s')\n",
            line_num, msg, symCour.nom);
    exit(EXIT_FAILURE);
}

void LireCar()
{
    car_cour = fgetc(fsource);
    if (car_cour == '\n') {
        line_num++;
    }
}

// Convert string to lowercase in-place
static void toLowerString(char *s)
{
    for(int i = 0; s[i]; i++) {
        s[i] = (char)tolower((unsigned char)s[i]);
    }
}

// Read a number (int or real)
static void lireNombre()
{
    char buf[64];
    int  i = 0;
    int  isReal = 0;

    while (i < 63 && (isdigit(car_cour) || car_cour == '.')) {
        if (car_cour == '.') {
            if (isReal) {
                // second '.' => break
                break;
            }
            isReal = 1;
        }
        buf[i++] = (char)car_cour;
        LireCar();
    }
    buf[i] = '\0';

    strncpy(symCour.nom, buf, sizeof(symCour.nom)-1);
    symCour.nom[sizeof(symCour.nom)-1] = '\0';
    symCour.cls = (isReal ? REAL_TOKEN : NUM_TOKEN);
}

static void lireMot()
{
    int i = 0;
    while ((isalnum(car_cour) || car_cour=='_') && i<63) {
        symCour.nom[i++] = (char)car_cour;
        LireCar();
    }
    symCour.nom[i] = '\0';
    toLowerString(symCour.nom);

    // Check keywords
    if      (!strcmp(symCour.nom,"program"))   symCour.cls = PROGRAM_TOKEN;
    else if (!strcmp(symCour.nom,"var"))       symCour.cls = VAR_TOKEN;
    else if (!strcmp(symCour.nom,"const"))     symCour.cls = CONST_TOKEN;
    else if (!strcmp(symCour.nom,"type"))      symCour.cls = TYPE_TOKEN;
    else if (!strcmp(symCour.nom,"if"))        symCour.cls = IF_TOKEN;
    else if (!strcmp(symCour.nom,"then"))      symCour.cls = THEN_TOKEN;
    else if (!strcmp(symCour.nom,"else"))      symCour.cls = ELSE_TOKEN;
    else if (!strcmp(symCour.nom,"while"))     symCour.cls = WHILE_TOKEN;
    else if (!strcmp(symCour.nom,"do"))        symCour.cls = DO_TOKEN;
    else if (!strcmp(symCour.nom,"repeat"))    symCour.cls = REPEAT_TOKEN;
    else if (!strcmp(symCour.nom,"until"))     symCour.cls = UNTIL_TOKEN;
    else if (!strcmp(symCour.nom,"for"))       symCour.cls = FOR_TOKEN;
    else if (!strcmp(symCour.nom,"to"))        symCour.cls = TO_TOKEN;
    else if (!strcmp(symCour.nom,"downto"))    symCour.cls = DOWNTO_TOKEN;
    else if (!strcmp(symCour.nom,"case"))      symCour.cls = CASE_TOKEN;
    else if (!strcmp(symCour.nom,"of"))        symCour.cls = OF_TOKEN;
    else if (!strcmp(symCour.nom,"begin"))     symCour.cls = BEGIN_TOKEN;
    else if (!strcmp(symCour.nom,"end"))       symCour.cls = END_TOKEN;
    else if (!strcmp(symCour.nom,"write"))     symCour.cls = WRITE_TOKEN;
    else if (!strcmp(symCour.nom,"read"))      symCour.cls = READ_TOKEN;
    else if (!strcmp(symCour.nom,"procedure")) symCour.cls = PROCEDURE_TOKEN;
    else if (!strcmp(symCour.nom,"function"))  symCour.cls = FUNCTION_TOKEN;
    else if (!strcmp(symCour.nom,"integer"))   symCour.cls = INT_TOKEN;
    else if (!strcmp(symCour.nom,"real"))      symCour.cls = REAL_TOKEN;
    else if (!strcmp(symCour.nom,"boolean"))   symCour.cls = BOOL_TOKEN;
    else if (!strcmp(symCour.nom,"string"))    symCour.cls = STRING_TOKEN;
    else                                       symCour.cls = ID_TOKEN;
}

void SymSuiv()
{
    // skip whitespace
    while (isspace(car_cour)) {
        LireCar();
    }

    if (car_cour == EOF || car_cour == -1) {
        symCour.cls = DIEZE_TOKEN;
        strcpy(symCour.nom,"#EOF");
        return;
    }

    if (isalpha(car_cour)) {
        lireMot();
    }
    else if (isdigit(car_cour)) {
        lireNombre();
    }
    else {
        switch(car_cour) {
        case '+': symCour.cls=PLUS_TOKEN;  symCour.nom[0]='+'; symCour.nom[1]='\0'; LireCar(); break;
        case '-': symCour.cls=MOINS_TOKEN; symCour.nom[0]='-'; symCour.nom[1]='\0'; LireCar(); break;
        case '*': symCour.cls=MULTI_TOKEN; symCour.nom[0]='*'; symCour.nom[1]='\0'; LireCar(); break;
        case '/': symCour.cls=DIV_TOKEN;   symCour.nom[0]='/'; symCour.nom[1]='\0'; LireCar(); break;
        case ';': symCour.cls=PV_TOKEN;    symCour.nom[0]=';'; symCour.nom[1]='\0'; LireCar(); break;
        case '.': symCour.cls=PT_TOKEN;    symCour.nom[0]='.'; symCour.nom[1]='\0'; LireCar(); break;
        case '(': symCour.cls=PRG_TOKEN;   symCour.nom[0]='(' ;symCour.nom[1]='\0'; LireCar(); break;
        case ')': symCour.cls=PRD_TOKEN;   symCour.nom[0]=')'; symCour.nom[1]='\0'; LireCar(); break;
        case '=': symCour.cls=EGAL_TOKEN;  symCour.nom[0]='='; symCour.nom[1]='\0'; LireCar(); break;
        case ',': symCour.cls=VIR_TOKEN;   symCour.nom[0]=','; symCour.nom[1]='\0'; LireCar(); break;

        case '<':
            LireCar();
            if (car_cour == '=') {
                symCour.cls=INFEG_TOKEN; strcpy(symCour.nom,"<=");
                LireCar();
            }
            else if (car_cour == '>') {
                symCour.cls=DIFF_TOKEN; strcpy(symCour.nom,"<>");
                LireCar();
            }
            else {
                symCour.cls=INF_TOKEN; strcpy(symCour.nom,"<");
            }
            break;

        case '>':
            LireCar();
            if (car_cour == '=') {
                symCour.cls=SUPEG_TOKEN; strcpy(symCour.nom,">=");
                LireCar();
            }
            else {
                symCour.cls=SUP_TOKEN; strcpy(symCour.nom,">");
            }
            break;

        case ':':
            LireCar();
            if (car_cour == '=') {
                symCour.cls=AFFECT_TOKEN; strcpy(symCour.nom,":=");
                LireCar();
            }
            else {
                symCour.cls=COLON_TOKEN; strcpy(symCour.nom,":");
            }
            break;

        default:
            symCour.cls = ERREUR_TOKEN;
            sprintf(symCour.nom, "%c", car_cour);
            Error("Unknown character");
            break;
        }
    }
}

// testSym() => checks the current token is the expected one, then consumes it
void testSym(TokenType t)
{
    if (symCour.cls == t) {
        symPre = symCour;
        SymSuiv();
    }
    else {
        char buf[128];
        sprintf(buf, "Unexpected token. Expected %d, found %d", t, symCour.cls);
        Error(buf);
    }
}
