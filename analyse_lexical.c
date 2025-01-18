#include "analyse_lexical.h"

// Token courant qui contient le type et la chaîne associée
TSym_Cour symCour;       

// Token précédent, utilisé pour garder en mémoire le dernier token consommé
TSym_Cour symPre;        

// Fichier source à analyser (souvent ouvert avant l'analyse)
FILE*     fsource = NULL;

// Caractère courant lu dans le fichier source
int       car_cour = 0;

// Numéro de la ligne en cours (pour l'affichage d'erreurs par exemple)
int       line_num = 1;

// Affiche une erreur avec le numéro de ligne et le token qui pose problème, puis quitte le programme
void Error(const char* msg)
{
    fprintf(stderr, "Error line %d: %s (last token: '%s')\n",
            line_num, msg, symCour.nom);
    exit(EXIT_FAILURE);
}

// Lit le caractère suivant dans le fichier source et met à jour le numéro de ligne si nécessaire
void LireCar()
{
    car_cour = fgetc(fsource);
    if (car_cour == '\n') {
        line_num++;  // On passe à la ligne suivante
    }
}

// Convertit une chaîne en minuscules, modifiant la chaîne directement
static void toLowerString(char *s)
{
    for(int i = 0; s[i]; i++) {
        s[i] = (char)tolower((unsigned char)s[i]);  // Convertit chaque caractère en minuscule
    }
}

// Lit un nombre depuis le flux (peut être un entier ou un réel)
// Stocke le résultat dans symCour.nom et définit le type de token (NUM_TOKEN ou REAL_TOKEN)
static void lireNombre()
{
    char buf[64];   // Buffer pour construire la chaîne représentant le nombre
    int  i = 0;     // Position actuelle dans le buffer
    int  isReal = 0;  // Indique si le nombre contient un point (donc réel)

    // Tant que le caractère courant est un chiffre ou un point et qu'on ne dépasse pas la taille du buffer
    while (i < 63 && (isdigit(car_cour) || car_cour == '.')) {
        if (car_cour == '.') {
            if (isReal) {
                // Si un point a déjà été trouvé, on arrête la lecture
                break;
            }
            isReal = 1;  // Marque que c'est un réel
        }
        buf[i++] = (char)car_cour;  // Stocke le caractère dans le buffer
        LireCar();  // Lit le caractère suivant
    }
    buf[i] = '\0';  // Termine la chaîne

    // Copie la chaîne dans le token courant
    strncpy(symCour.nom, buf, sizeof(symCour.nom)-1);
    symCour.nom[sizeof(symCour.nom)-1] = '\0';

    // Définit le type du token en fonction de la présence d'un point
    symCour.cls = (isReal ? REAL_TOKEN : NUM_TOKEN);
}

// Lit un mot (identifiant ou mot-clé) depuis le flux d'entrée
static void lireMot()
{
    int i = 0;  // Index pour construire le mot
    // Tant que le caractère est alphanumérique ou un '_' et que le buffer n'est pas plein
    while ((isalnum(car_cour) || car_cour=='_') && i < 63) {
        symCour.nom[i++] = (char)car_cour;  // Ajoute le caractère au mot
        LireCar();  // Lit le caractère suivant
    }
    symCour.nom[i] = '\0';  // Termine le mot par un caractère nul
    toLowerString(symCour.nom);  // Met le mot en minuscules pour faciliter la comparaison

    // Vérifie si le mot correspond à un mot-clé connu et définit le type du token
    if      (!strcmp(symCour.nom, "program"))   symCour.cls = PROGRAM_TOKEN;
    else if (!strcmp(symCour.nom, "var"))       symCour.cls = VAR_TOKEN;
    else if (!strcmp(symCour.nom, "const"))     symCour.cls = CONST_TOKEN;
    else if (!strcmp(symCour.nom, "type"))      symCour.cls = TYPE_TOKEN;
    else if (!strcmp(symCour.nom, "if"))        symCour.cls = IF_TOKEN;
    else if (!strcmp(symCour.nom, "then"))      symCour.cls = THEN_TOKEN;
    else if (!strcmp(symCour.nom, "else"))      symCour.cls = ELSE_TOKEN;
    else if (!strcmp(symCour.nom, "while"))     symCour.cls = WHILE_TOKEN;
    else if (!strcmp(symCour.nom, "do"))        symCour.cls = DO_TOKEN;
    else if (!strcmp(symCour.nom, "repeat"))    symCour.cls = REPEAT_TOKEN;
    else if (!strcmp(symCour.nom, "until"))     symCour.cls = UNTIL_TOKEN;
    else if (!strcmp(symCour.nom, "for"))       symCour.cls = FOR_TOKEN;
    else if (!strcmp(symCour.nom, "to"))        symCour.cls = TO_TOKEN;
    else if (!strcmp(symCour.nom, "downto"))    symCour.cls = DOWNTO_TOKEN;
    else if (!strcmp(symCour.nom, "case"))      symCour.cls = CASE_TOKEN;
    else if (!strcmp(symCour.nom, "of"))        symCour.cls = OF_TOKEN;
    else if (!strcmp(symCour.nom, "begin"))     symCour.cls = BEGIN_TOKEN;
    else if (!strcmp(symCour.nom, "end"))       symCour.cls = END_TOKEN;
    else if (!strcmp(symCour.nom, "write"))     symCour.cls = WRITE_TOKEN;
    else if (!strcmp(symCour.nom, "read"))      symCour.cls = READ_TOKEN;
    else if (!strcmp(symCour.nom, "procedure")) symCour.cls = PROCEDURE_TOKEN;
    else if (!strcmp(symCour.nom, "function"))  symCour.cls = FUNCTION_TOKEN;
    else if (!strcmp(symCour.nom, "integer"))   symCour.cls = INT_TOKEN;
    else if (!strcmp(symCour.nom, "real"))      symCour.cls = REAL_TOKEN;
    else if (!strcmp(symCour.nom, "boolean"))   symCour.cls = BOOL_TOKEN;
    else if (!strcmp(symCour.nom, "string"))    symCour.cls = STRING_TOKEN;
    else                                       symCour.cls = ID_TOKEN;  // Sinon, c'est un identifiant
}

// Passe au prochain symbole (token) dans le flux d'entrée
void SymSuiv()
{
    // Ignore tous les espaces, retours à la ligne, etc.
    while (isspace(car_cour)) {
        LireCar();  // Lit le prochain caractère jusqu'à trouver un non-espace
    }

    // Si on atteint la fin du fichier, définit un token spécial de fin
    if (car_cour == EOF || car_cour == -1) {
        symCour.cls = DIEZE_TOKEN;
        strcpy(symCour.nom, "#EOF");
        return;
    }

    // Si le caractère est une lettre, le mot (identifiant ou mot-clé) est lu
    if (isalpha(car_cour)) {
        lireMot();
    }
    // Si le caractère est un chiffre, lit un nombre
    else if (isdigit(car_cour)) {
        lireNombre();
    }
    else {
        // Sinon, c'est un symbole ou un opérateur
        switch(car_cour) {
        case '+':
            symCour.cls = PLUS_TOKEN;
            symCour.nom[0] = '+';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case '-':
            symCour.cls = MOINS_TOKEN;
            symCour.nom[0] = '-';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case '*':
            symCour.cls = MULTI_TOKEN;
            symCour.nom[0] = '*';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case '/':
            symCour.cls = DIV_TOKEN;
            symCour.nom[0] = '/';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case ';':
            symCour.cls = PV_TOKEN;
            symCour.nom[0] = ';';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case '.':
            symCour.cls = PT_TOKEN;
            symCour.nom[0] = '.';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case '(':
            symCour.cls = PRG_TOKEN;
            symCour.nom[0] = '(';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case ')':
            symCour.cls = PRD_TOKEN;
            symCour.nom[0] = ')';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case '=':
            symCour.cls = EGAL_TOKEN;
            symCour.nom[0] = '=';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case ',':
            symCour.cls = VIR_TOKEN;
            symCour.nom[0] = ',';
            symCour.nom[1] = '\0';
            LireCar();
            break;
        case '<':
            LireCar();  // Passe au caractère suivant pour vérifier le type
            if (car_cour == '=') {
                symCour.cls = INFEG_TOKEN;
                strcpy(symCour.nom, "<=");
                LireCar();
            }
            else if (car_cour == '>') {
                symCour.cls = DIFF_TOKEN;
                strcpy(symCour.nom, "<>");
                LireCar();
            }
            else {
                symCour.cls = INF_TOKEN;
                strcpy(symCour.nom, "<");
            }
            break;
        case '>':
            LireCar();
            if (car_cour == '=') {
                symCour.cls = SUPEG_TOKEN;
                strcpy(symCour.nom, ">=");
                LireCar();
            }
            else {
                symCour.cls = SUP_TOKEN;
                strcpy(symCour.nom, ">");
            }
            break;
        case ':':
            LireCar();
            if (car_cour == '=') {
                symCour.cls = AFFECT_TOKEN;
                strcpy(symCour.nom, ":=");
                LireCar();
            }
            else {
                symCour.cls = COLON_TOKEN;
                strcpy(symCour.nom, ":");
            }
            break;
        default:
            // Si aucun cas ne correspond, c'est un caractère inconnu
            symCour.cls = ERREUR_TOKEN;
            sprintf(symCour.nom, "%c", car_cour);
            Error("Unknown character");
            break;
        }
    }
}

// Vérifie que le token courant correspond au token attendu, puis passe au suivant
void testSym(TokenType t)
{
    if (symCour.cls == t) {
        symPre = symCour;  // Sauvegarde le token courant en tant que token précédent
        SymSuiv();         // Passe au token suivant
    }
    else {
        char buf[128];
        sprintf(buf, "Unexpected token. Expected %d, found %d", t, symCour.cls);
        Error(buf);  // Affiche une erreur si le token ne correspond pas
    }
}
