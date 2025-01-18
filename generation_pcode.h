#ifndef GENERATION_PCODE_H
#define GENERATION_PCODE_H

#include "global.h"

void Ecrire1(Mnemoniques M);
void Ecrire2(Mnemoniques M, int arg);

void afficherPCode();
void sauvegarderPCode(const char* filename);
void chargerPCode(const char* filename);

#endif
