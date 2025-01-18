#ifndef SEMANTIQUE_H
#define SEMANTIQUE_H

#include "global.h"  // Inclut les définitions globales (types, constantes, etc.)

// ----------------------
// Type d'entrée (genre)
// ----------------------
// Enumération qui définit le genre d'un identifiant dans la table des symboles
typedef enum {
    TVAR,    // Identifiant est une variable
    TCONST,  // Identifiant est une constante
    TTYPE,   // Identifiant est un type déclaré par l'utilisateur
    TPROC,   // Identifiant est une procédure
    TFUNC    // Identifiant est une fonction
} TTypeIDF;  // Permet de savoir quel genre d'élément est enregistré

// ----------------------
// Entrée de la table des symboles
// ----------------------
// Structure qui stocke les informations sur un identifiant
typedef struct {
    char     Nom[32];   // Nom de l'identifiant (maximum 31 caractères + fin de chaîne)
    TTypeIDF TIDF;      // Genre de l'identifiant (variable, constante, type, procédure ou fonction)
    DataType type;      // Type effectif (TYPE_INT, TYPE_REAL, etc.) pour les variables et constantes
    int      Adresse;   // Adresse en mémoire ou adresse dans le code (pour les procédures/fonctions)
    int      Value;     // Pour une constante : sa valeur entière, ou pour les proc/func : le nombre de paramètres
    float    FValue;    // Pour une constante réelle : sa valeur en float
} T_IDF;  // Chaque entrée représente un identifiant de la table des symboles

// Tableau global qui contient les entrées de la table des symboles
extern T_IDF TAB_IDFS[TAILLEIDFS];  // Tableau des identifiants, avec une taille maximale donnée par TAILLEIDFS
extern int   NBR_IDFS;              // Nombre d'entrées actuellement dans la table des symboles
extern int   OFFSET;                // Prochaine adresse mémoire globale disponible pour les variables

// ----------------------
// Fonctions de vérification de la table des symboles
// ----------------------

// Vérifie si un identifiant existe déjà dans la table
int IDexists(const char* nom);

// Vérifie si l'identifiant est une variable
int isVar(const char* nom);

// Vérifie si l'identifiant est une constante
int isConst(const char* nom);

// Retourne l'adresse en mémoire d'une variable
int getAdresse(const char* nom);

// Retourne la valeur entière d'une constante
int getConstValue(const char* nom);

// Retourne la valeur réelle (float) d'une constante
float getConstFValue(const char* nom);

// Vérifie si l'identifiant correspond à une procédure
int isProcedure(const char* nom);

// Vérifie si l'identifiant correspond à une fonction
int isFunction(const char* nom);

// Retourne l'index de l'entrée d'une procédure ou fonction dans la table des symboles
int getProcFuncIndex(const char* nom);

// ----------------------
// Déclarations pour la partie sémantique (analyser les déclarations)
// ----------------------

// Analyse et traite les déclarations de types dans le code source
void TypeDecl();

// Analyse et traite les déclarations de constantes dans le code source
void ConstDecl();

// Analyse et traite les déclarations de variables dans le code source
void VarDecl();

// Parse et retourne un type de base (ex : int, real, etc.) pour les déclarations de variables ou de paramètres
DataType parseBaseType();

#endif
