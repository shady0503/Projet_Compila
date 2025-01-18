#include "semantique.h"
#include "analyse_lexical.h"

// Tableau global pour stocker les entrées de la table des symboles
T_IDF TAB_IDFS[TAILLEIDFS];
// Nombre d'entrées actuellement enregistrées dans la table des symboles
int NBR_IDFS = 0;
// Adresse globale suivante pour déclarer une variable
int OFFSET = VAR_BASE;

// ---------------------------------------------------------------------
// Vérifie si un identifiant existe déjà dans la table des symboles
// Retourne 1 si trouvé, sinon 0
// ---------------------------------------------------------------------
int IDexists(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom))
            return 1; // Identifiant trouvé
    }
    return 0; // Non trouvé
}

// ---------------------------------------------------------------------
// Vérifie si un identifiant est une variable (TVAR)
// ---------------------------------------------------------------------
int isVar(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TVAR)
            return 1; // C'est une variable
    }
    return 0; // Ce n'est pas une variable
}

// ---------------------------------------------------------------------
// Vérifie si un identifiant est une constante (TCONST)
// ---------------------------------------------------------------------
int isConst(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TCONST)
            return 1; // C'est une constante
    }
    return 0; // Ce n'est pas une constante
}

// ---------------------------------------------------------------------
// Retourne l'adresse en mémoire d'une variable (TVAR)
// Si non trouvée, affiche une erreur
// ---------------------------------------------------------------------
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

// ---------------------------------------------------------------------
// Retourne la valeur entière d'une constante de type entier (TYPE_INT)
// ---------------------------------------------------------------------
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

// ---------------------------------------------------------------------
// Retourne la valeur réelle (float) d'une constante de type réel (TYPE_REAL)
// ---------------------------------------------------------------------
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

// ---------------------------------------------------------------------
// Vérifie si l'identifiant est une procédure (TPROC)
// ---------------------------------------------------------------------
int isProcedure(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TPROC)
            return 1; // C'est une procédure
    }
    return 0;
}

// ---------------------------------------------------------------------
// Vérifie si l'identifiant est une fonction (TFUNC)
// ---------------------------------------------------------------------
int isFunction(const char *nom)
{
    for (int i = 0; i < NBR_IDFS; i++)
    {
        if (!strcmp(TAB_IDFS[i].Nom, nom) && TAB_IDFS[i].TIDF == TFUNC)
            return 1; // C'est une fonction
    }
    return 0;
}

// ---------------------------------------------------------------------
// Retourne l'index d'une procédure ou fonction dans la table des symboles
// Si non trouvé, affiche une erreur
// ---------------------------------------------------------------------
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

// ---------------------------------------------------------------------
// Déclarations de type (alias)
// Exemple : a, b = integer;
// ---------------------------------------------------------------------
void TypeDecl()
{
    // Tant que le symbole courant est un identifiant
    while (symCour.cls == ID_TOKEN)
    {
        char listIDS[10][32]; // Stocke jusqu'à 10 alias dans une ligne
        int n = 0;
        // Récupère un groupe d'identifiants séparés par des virgules
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

        testSym(EGAL_TOKEN); // Attend le signe '='

        DataType d = TYPE_UNDEF;
        // Vérifie et fixe le type de base (integer, real, boolean, string)
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

        // Pour chaque identifiant de la liste, l'ajoute dans la table des symboles
        for (int i = 0; i < n; i++)
        {
            if (IDexists(listIDS[i]))
                Error("Alias name already used");
            strcpy(TAB_IDFS[NBR_IDFS].Nom, listIDS[i]);
            TAB_IDFS[NBR_IDFS].TIDF = TTYPE;  // C'est un alias de type
            TAB_IDFS[NBR_IDFS].type = d;
            TAB_IDFS[NBR_IDFS].Adresse = -1;   // Pas d'adresse associée
            NBR_IDFS++; // Augmente le nombre d'identifiants enregistrés
        }
        // Si le symbole courant est un point-virgule, le consomme, sinon sort de la boucle
        if (symCour.cls == PV_TOKEN)
            testSym(PV_TOKEN);
        else
            break;
    }
}

// ---------------------------------------------------------------------
// Déclarations de constantes
// Exemple : const a = 10;
// ---------------------------------------------------------------------
void ConstDecl()
{
    // Tant que le symbole courant est un identifiant
    while (symCour.cls == ID_TOKEN)
    {
        char nom[32];
        strcpy(nom, symCour.nom);
        testSym(ID_TOKEN);
        testSym(EGAL_TOKEN);

        // Si le nom existe déjà, c'est une erreur
        if (IDexists(nom))
            Error("Const name declared twice");
        strcpy(TAB_IDFS[NBR_IDFS].Nom, nom);
        TAB_IDFS[NBR_IDFS].TIDF = TCONST; // Marque comme constante
        TAB_IDFS[NBR_IDFS].Adresse = -1;  // Pas d'adresse requise pour une constante

        // La constante doit être numérique : entier ou réel
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
        NBR_IDFS++;     // Incrémente le nombre d'entrées dans la table des symboles
        testSym(PV_TOKEN); // Attend et consomme le point-virgule
    }
}

// ---------------------------------------------------------------------
// Déclarations de variables
// Exemple : var a, b: integer;
// ---------------------------------------------------------------------
void VarDecl()
{
    // Tant que le symbole courant est un identifiant
    while (symCour.cls == ID_TOKEN)
    {
        char listIDS[10][32]; // Stocke jusqu'à 10 variables dans une ligne
        int n = 0;
        // Récupère la liste d'identifiants séparés par des virgules
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

        DataType declaredType = TYPE_INT; // Par défaut, on suppose qu'il s'agit d'un entier
        if (symCour.cls == COLON_TOKEN)
        {
            testSym(COLON_TOKEN);
            declaredType = parseBaseType(); // Analyse le type de base fourni
        }

        testSym(PV_TOKEN); // Attend et consomme le point-virgule

        // Pour chaque identifiant, le rajoute dans la table des symboles
        for (int i = 0; i < n; i++)
        {
            if (IDexists(listIDS[i]))
                Error("Var name used");
            strcpy(TAB_IDFS[NBR_IDFS].Nom, listIDS[i]);
            TAB_IDFS[NBR_IDFS].TIDF = TVAR;      // Marque comme variable
            TAB_IDFS[NBR_IDFS].type = declaredType; // Assigne le type déclaré

            // Assigne la prochaine adresse mémoire disponible et l'incrémente
            TAB_IDFS[NBR_IDFS].Adresse = OFFSET++;
            printf("Declared variable: %s, Type: %d, Address: %d\n", 
                   TAB_IDFS[NBR_IDFS].Nom, TAB_IDFS[NBR_IDFS].type, TAB_IDFS[NBR_IDFS].Adresse);
            NBR_IDFS++; // Incrémente le nombre d'entrées
        }
    }
}

// ---------------------------------------------------------------------
// Analyse et retourne un type de base pour les déclarations de variables ou paramètres
// Les types de base possibles sont: int, real, boolean, string, mais fonctionnel dans le code que int , et real, baraka hhhhh
// ---------------------------------------------------------------------
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
