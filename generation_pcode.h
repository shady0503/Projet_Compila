#ifndef GENERATION_PCODE_H
#define GENERATION_PCODE_H

#include "global.h"  // On inclut les définitions globales comme Mnemoniques, PCODE, PC, etc.

// ---------------------------------------------------------------------
// Ecrire1 : écrit une instruction P-code sans argument
// ---------------------------------------------------------------------
// Paramètre M : le mnémonique (instruction) à écrire
void Ecrire1(Mnemoniques M);

// ---------------------------------------------------------------------
// Ecrire2 : écrit une instruction P-code avec un argument entier
// ---------------------------------------------------------------------
// Paramètre M : le mnémonique (instruction) à écrire
// Paramètre arg : l'argument entier associé à l'instruction
void Ecrire2(Mnemoniques M, int arg);

// ---------------------------------------------------------------------
// afficherPCode : affiche toutes les instructions du P-code
// ---------------------------------------------------------------------
// Affiche le contenu du tableau global PCODE pour voir le programme généré
void afficherPCode();

// ---------------------------------------------------------------------
// sauvegarderPCode : sauvegarde le P-code dans un fichier
// ---------------------------------------------------------------------
// Paramètre filename : le nom du fichier où enregistrer le P-code
void sauvegarderPCode(const char* filename);

// ---------------------------------------------------------------------
// chargerPCode : charge le P-code depuis un fichier
// ---------------------------------------------------------------------
// Paramètre filename : le nom du fichier à partir duquel charger le P-code
void chargerPCode(const char* filename);

#endif
