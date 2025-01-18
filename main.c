#include "global.h"            // Définitions globales (constantes, types, etc.)
#include "analyse_lexical.h"   // Fonctions d'analyse lexicale (LireCar, SymSuiv, etc.)
#include "syntaxique.h"        // Fonctions d'analyse syntaxique (Program, etc.)
#include "semantique.h"        // Fonctions d'analyse sémantique (ConstDecl, VarDecl, etc.)
#include "generation_pcode.h"  // Fonctions pour générer le P-code (Ecrire1, Ecrire2, etc.)
#include "interpreteur.h"      // Interpréteur de P-code (INTER_PCODE)

int main(int argc, char* argv[])
{
    // Vérifie que l'utilisateur a fourni au moins un argument (fichier source)
    if(argc < 2){
        printf("Usage: %s <source_file> [pcode_file]\n", argv[0]); // Message d'usage
        return 1;
    }

    // Ouvre le fichier source en mode lecture
    fsource = fopen(argv[1], "r");
    if(!fsource){
        perror("fopen source"); // Affiche l'erreur si le fichier source ne peut être ouvert
        return 1;
    }

    // Initialise la lecture du flux source
    LireCar();   // Lit le premier caractère du fichier source
    SymSuiv();   // Analyse et charge le premier symbole/token

    // Lance l'analyse syntaxique du programme source.
    // La fonction Program() va analyser le code source et générer le P-code.
    Program(); // => Remplit PCODE avec les instructions

    // Initialise MEM_TYPE pour chaque variable globale à partir de la table des symboles.
    // Pour chaque variable enregistrée, on définit son type dans le tableau MEM_TYPE.
    for(int i = 0; i < NBR_IDFS; i++){
        if(TAB_IDFS[i].TIDF == TVAR){
            int adr = TAB_IDFS[i].Adresse;
            if(adr >= 0 && adr < TAILLEMEM){
                MEM_TYPE[adr] = TAB_IDFS[i].type;
            }
        }
    }

    // Affiche un message de succès de compilation et le nombre d'instructions générées.
    printf("Compilation successful. PC=%d\n", PC);
    afficherPCode(); // Affiche le P-code généré pour le débogage

    // Si un deuxième argument est fourni, sauvegarde le P-code dans un fichier,
    // puis recharge le P-code depuis ce fichier.
    if(argc > 2){
        sauvegarderPCode(argv[2]);  // Sauvegarde le P-code dans le fichier spécifié
        fclose(fsource);            // Ferme le fichier source
        chargerPCode(argv[2]);      // Recharge le P-code à partir du fichier
    } else {
        fclose(fsource);            // Ferme le fichier source s'il n'y a pas de sauvegarde
    }

    // Exécution du P-code à l'aide de l'interpréteur
    INTER_PCODE();

    return 0; // Fin du programme
}
