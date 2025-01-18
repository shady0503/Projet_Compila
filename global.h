#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>    // Pour faire des entrées/sorties (printf, scanf, etc.)
#include <stdlib.h>   // Pour les fonctions d'allocation mémoire, exit, etc.
#include <string.h>   // Pour manipuler les chaînes de caractères (strcpy, strcmp, etc.)
#include <ctype.h>    // Pour tester des caractères (isalpha, isdigit, etc.)

// Définit la taille maximale du tableau d'instructions du P-code
#define TAILLECODE 500       // Nombre maximal d'instructions
// Définit la taille de la mémoire allouée pour le stockage des valeurs
#define TAILLEMEM  500       // Taille de la mémoire globale
// Définit la taille maximale du tableau des identifiants
#define TAILLEIDFS 200       // Nombre maximal d'identifiants
// Définit la base d'adresse pour les variables
#define VAR_BASE   200       // Base pour les variables

// -------------------------------
// Déclaration des types de données
// -------------------------------

// Enumération simple pour les différents types de données du langage
typedef enum {
    TYPE_INT,    // Entier
    TYPE_REAL,   // Réel (nombre à virgule)
    TYPE_BOOL,   // Booléen (vrai ou faux)
    TYPE_STRING, // Chaîne de caractères
    TYPE_UNDEF   // Non défini
} DataType; // Permet de définir le type d'une donnée dans le programme

// Union pour stocker soit un entier, soit un réel dans une seule variable
typedef union {
    int   i;     // Stocke un entier
    float f;     // Stocke un nombre réel
} DataValue;   // Permet de garder soit un entier soit un réel

// -------------------------------
// Variables globales et tableaux
// -------------------------------

// Tableau global pour stocker les valeurs (mémoire du programme)
extern DataValue MEM[TAILLEMEM];  
// Tableau qui définit le type de chaque élément dans MEM
extern DataType  MEM_TYPE[TAILLEMEM];  
// Pointeur de pile (indique le sommet de la pile)
extern int       SP;  
// Pointeur de base pour les appels de fonctions/procédures
extern int       BP;  

// -------------------------------
// Définition des tokens du langage
// -------------------------------

// Enumération listant tous les tokens (mots-clés ou symboles) du langage
typedef enum {
    PROGRAM_TOKEN,     // Mot-clé 'program'
    VAR_TOKEN,         // Mot-clé 'var'
    CONST_TOKEN,       // Mot-clé 'const'
    TYPE_TOKEN,        // Mot-clé 'type'
    IF_TOKEN,          // Instruction 'if'
    THEN_TOKEN,        // Mot-clé 'then'
    ELSE_TOKEN,        // Mot-clé 'else'
    WHILE_TOKEN,       // Instruction 'while'
    DO_TOKEN,          // Mot-clé 'do'
    REPEAT_TOKEN,      // Instruction 'repeat'
    UNTIL_TOKEN,       // Mot-clé 'until'
    FOR_TOKEN,         // Instruction 'for'
    TO_TOKEN,          // Mot-clé 'to'
    DOWNTO_TOKEN,      // Mot-clé 'downto'
    CASE_TOKEN,        // Instruction 'case'
    OF_TOKEN,          // Mot-clé 'of'
    BEGIN_TOKEN,       // Début d'un bloc (mot-clé 'begin')
    END_TOKEN,         // Fin d'un bloc (mot-clé 'end')
    WRITE_TOKEN,       // Instruction 'write' pour afficher
    READ_TOKEN,        // Instruction 'read' pour lire
    PROCEDURE_TOKEN,   // Mot-clé 'procedure'
    FUNCTION_TOKEN,    // Mot-clé 'function'
    AFFECT_TOKEN,      // Symbole d'affectation ':='
    PV_TOKEN,          // Point-virgule ';'
    PT_TOKEN,          // Point '.'
    PLUS_TOKEN,        // Signe plus '+'
    MOINS_TOKEN,       // Signe moins '-'
    MULTI_TOKEN,       // Signe multiplication '*'
    DIV_TOKEN,         // Signe division '/'
    EGAL_TOKEN,        // Signe égal '='
    DIFF_TOKEN,        // Signe différence '<>'
    SUP_TOKEN,         // Signe supérieur '>'
    INF_TOKEN,         // Signe inférieur '<'
    SUPEG_TOKEN,       // Signe supérieur ou égal '>='
    INFEG_TOKEN,       // Signe inférieur ou égal '<='
    COLON_TOKEN,       // Deux-points ':'
    PRG_TOKEN,         // Parenthèse ouvrante '('
    PRD_TOKEN,         // Parenthèse fermante ')'
    VIR_TOKEN,         // Virgule ','
    NUM_TOKEN,         // Nombre entier
    REAL_TOKEN,        // Nombre réel
    ID_TOKEN,          // Identifiant (nom de variable)
    INT_TOKEN,         // Mot-clé 'int'
    BOOL_TOKEN,        // Mot-clé 'bool'
    STRING_TOKEN,      // Mot-clé 'string'
    DIEZE_TOKEN,       // Symbole '#' souvent utilisé pour marquer la fin
    ERREUR_TOKEN       // Token indiquant une erreur
} TokenType; // Définit le type de chaque token dans le lexeur

// Structure représentant un token courant du compilateur
typedef struct {
    TokenType cls;      // Classe ou type du token
    char      nom[64];  // Nom ou valeur du token sous forme de chaîne
} TSym_Cour; // Utilisé pour stocker le token en cours d'analyse

extern TSym_Cour symCour;   // Le token actuellement analysé
extern TSym_Cour symPre;    // Le token précédent (pour suivi)
extern FILE*     fsource;   // Pointeur vers le fichier source à compiler
extern int       car_cour;  // Caractère courant dans le fichier source
extern int       line_num;  // Numéro de la ligne actuellement lue

// -------------------------------
// Définition des instructions du P-code
// -------------------------------

// Enumération listant toutes les instructions possibles dans le P-code
typedef enum {
    ADD,               // Addition
    SUB,               // Soustraction
    MUL,               // Multiplication
    DIVI,              // Division entière
    EQL,               // Égalité (==)
    NEQ,               // Différence (!=)
    GTR,               // Plus grand que (>)
    LSS,               // Plus petit que (<)
    GEQ,               // Supérieur ou égal (>=)
    LEQ,               // Inférieur ou égal (<=)
    PRN,               // Instruction d'impression (print)
    INN,               // Instruction de saisie (input)
    LDI,               // Charger une valeur immédiate (literal)
    LDA,               // Charger une adresse
    LDV,               // Charger la valeur depuis une adresse
    STO,               // Stocker une valeur dans une adresse
    BRN,               // Branchement inconditionnel (jump)
    BZE,               // Branchement conditionnel si zéro (if zero, jump)
    HLT,               // Arrêter le programme (halt)
    CALL,              // Appel d'une procédure ou fonction (call)
    RET,               // Retour d'une procédure ou fonction (return)
    LDL,               // Charger une variable locale
    STL,               // Stocker dans une variable locale
    LDF,               // Charger une fonction
    STO_IND,           // Stocker via une adresse indirecte
    PUSH_PARAMS_COUNT  // Pousser le nombre de paramètres sur la pile
} Mnemoniques; // Définit toutes les opérations possibles en P-code

// Structure qui représente une instruction du P-code
typedef struct {
    Mnemoniques MNE; // Le mnémonique (opération) de l'instruction
    int         SUITE; // L'argument ou suite d'instruction associé
} INSTRUCTION; // Utilisé pour créer chaque instruction du P-code

extern INSTRUCTION PCODE[TAILLECODE]; // Tableau global contenant les instructions du P-code
extern int         PC;                // Compteur ou pointeur courant dans le tableau PCODE

// Déclaration d'une fonction pour afficher une erreur et peut-être arrêter le programme
void Error(const char *msg); // Affiche le message d'erreur passé en paramètre

#endif
