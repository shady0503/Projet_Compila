#include "global.h"
#include "analyse_lexical.h"
#include "syntaxique.h"
#include "semantique.h"
#include "generation_pcode.h"
#include "interpreteur.h"

int main(int argc, char* argv[])
{
    if(argc<2){
        printf("Usage: %s <source_file> [pcode_file]\n", argv[0]);
        return 1;
    }

    fsource = fopen(argv[1],"r");
    if(!fsource){
        perror("fopen source");
        return 1;
    }

    LireCar();
    SymSuiv();

    // Lance l'analyse syntaxique
    Program(); // => remplit PCODE

    // Initialise MEM_TYPE pour chaque variable globale
    for(int i=0; i<NBR_IDFS; i++){
        if(TAB_IDFS[i].TIDF==TVAR){
            int adr = TAB_IDFS[i].Adresse;
            if(adr>=0 && adr<TAILLEMEM){
                MEM_TYPE[adr] = TAB_IDFS[i].type;
            }
        }
    }

    printf("Compilation successful. PC=%d\n", PC);
    afficherPCode(); // Debug

    if(argc>2){
        sauvegarderPCode(argv[2]);
        fclose(fsource);
        chargerPCode(argv[2]);
    } else {
        fclose(fsource);
    }

    // Exécution
    INTER_PCODE();

    return 0;
}
