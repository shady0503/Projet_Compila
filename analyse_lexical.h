#ifndef ANALYSE_LEXICAL_H
#define ANALYSE_LEXICAL_H

#include "global.h"  // Inclut le fichier global pour accéder aux définitions communes

// ---------------
// Analyse Lexicale
// ---------------

// Fonction qui lit le caractère courant dans le fichier source.
// Elle permet d'avancer dans le flux de caractères.
void LireCar();  // Lit le caractère suivant dans le flux d'entrée

// Fonction qui passe au symbole suivant.
// Elle met à jour le symbole courant pour l'analyse lexicale.
void SymSuiv();  // Met à jour le token courant en lisant le prochain symbole

// Fonction qui teste si le token courant correspond au token attendu.
// Paramètre t : le type de token attendu.
// Si le token courant n'est pas celui attendu, elle affiche une erreur.
void testSym(TokenType t);  // Vérifie que le token courant est bien du type spécifié

#endif
