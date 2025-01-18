#include "generation_pcode.h"
#include "semantique.h"

// Tableau global de P-code qui va stocker toutes les instructions générées
INSTRUCTION PCODE[TAILLECODE];
// PC (Program Counter) : index de la dernière instruction écrite dans PCODE
// Initialisé à -1 car aucune instruction n'a encore été écrite
int PC = -1;

// ---------------------------------------------------------------------
// Ecrire1 : Écrit une instruction sans argument dans le tableau PCODE
// ---------------------------------------------------------------------
// Paramètre M : le mnémonique (instruction) à ajouter
void Ecrire1(Mnemoniques M) {
    // Vérifie qu'on n'a pas dépassé la taille maximale du tableau
    if (PC >= TAILLECODE - 1)
        Error("Too many instructions");
    PC++;                   // Incrémente le compteur de programme
    PCODE[PC].MNE = M;      // Stocke le mnémonique dans l'instruction courante
    PCODE[PC].SUITE = 0;    // Aucun argument n'est nécessaire, on met 0
}

// ---------------------------------------------------------------------
// Ecrire2 : Écrit une instruction avec un argument entier dans PCODE
// ---------------------------------------------------------------------
// Paramètre M   : le mnémonique (instruction) à ajouter
// Paramètre arg : l'argument entier associé à l'instruction
void Ecrire2(Mnemoniques M, int arg) {
    // Vérifie qu'on n'a pas dépassé la capacité du tableau
    if (PC >= TAILLECODE - 1)
        Error("Too many instructions");
    PC++;                   // Passe à la prochaine position
    PCODE[PC].MNE = M;      // Assigne le mnémonique
    PCODE[PC].SUITE = arg;  // Stocke l'argument associé à l'instruction
}

// ---------------------------------------------------------------------
// afficherPCode : Affiche toutes les instructions du P-code
// ---------------------------------------------------------------------
// Parcourt le tableau PCODE et imprime pour chaque instruction son index,
// le mnémonique et l'argument
void afficherPCode() {
    for (int i = 0; i <= PC; i++) {
        printf("%3d : (%d, %d)\n", i, PCODE[i].MNE, PCODE[i].SUITE);
    }
}

// ---------------------------------------------------------------------
// sauvegarderPCode : Sauvegarde le P-code dans un fichier
// ---------------------------------------------------------------------
// Paramètre filename : le nom du fichier où le P-code sera sauvegardé
// Ouvre le fichier, écrit chaque instruction et ferme le fichier.
void sauvegarderPCode(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        perror("fopen");
        return;
    }
    for (int i = 0; i <= PC; i++) {
        // Écrit le mnémonique et l'argument de chaque instruction sur une ligne
        fprintf(f, "%d %d\n", PCODE[i].MNE, PCODE[i].SUITE);
    }
    fclose(f);
    printf("P-code saved in %s\n", filename);
}

// ---------------------------------------------------------------------
// chargerPCode : Charge le P-code à partir d'un fichier
// ---------------------------------------------------------------------
// Paramètre filename : le nom du fichier d'où charger le P-code
// Lit chaque instruction du fichier et les stocke dans PCODE, puis ferme le fichier.
void chargerPCode(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    PC = -1;  // Réinitialise le compteur de programme
    while (!feof(f)) {
        int m, s;
        // Lit un mnémonique et son argument depuis le fichier
        if (fscanf(f, "%d %d", &m, &s) == 2) {
            PC++;
            if (PC >= TAILLECODE)
                Error("P-code too large");
            PCODE[PC].MNE = (Mnemoniques)m; // Convertit en type Mnemoniques
            PCODE[PC].SUITE = s;
        }
    }
    fclose(f);
    printf("P-code loaded from %s (PC=%d)\n", filename, PC);
}
