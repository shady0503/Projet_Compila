#include "generation_pcode.h"
#include "semantique.h"

INSTRUCTION PCODE[TAILLECODE];
int         PC=-1;

void Ecrire1(Mnemoniques M){
    if(PC>=TAILLECODE-1) Error("Too many instructions");
    PC++;
    PCODE[PC].MNE = M;
    PCODE[PC].SUITE=0;
}
void Ecrire2(Mnemoniques M,int arg){
    if(PC>=TAILLECODE-1) Error("Too many instructions");
    PC++;
    PCODE[PC].MNE   = M;
    PCODE[PC].SUITE = arg;
}

void afficherPCode(){
    for(int i=0; i<=PC; i++){
        printf("%3d : (%d, %d)\n", i, PCODE[i].MNE, PCODE[i].SUITE);
    }
}

void sauvegarderPCode(const char* filename){
    FILE* f = fopen(filename,"w");
    if(!f){ perror("fopen"); return; }
    for(int i=0; i<=PC; i++){
        fprintf(f, "%d %d\n", PCODE[i].MNE, PCODE[i].SUITE);
    }
    fclose(f);
    printf("P-code saved in %s\n", filename);
}

void chargerPCode(const char* filename){
    FILE* f = fopen(filename,"r");
    if(!f){ perror("fopen"); exit(EXIT_FAILURE); }
    PC=-1;
    while(!feof(f)){
        int m, s;
        if(fscanf(f,"%d %d",&m,&s)==2){
            PC++;
            if(PC>=TAILLECODE) Error("P-code too large");
            PCODE[PC].MNE=(Mnemoniques)m;
            PCODE[PC].SUITE=s;
        }
    }
    fclose(f);
    printf("P-code loaded from %s (PC=%d)\n", filename, PC);
}
