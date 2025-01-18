#ifndef SYNTAXIQUE_H
#define SYNTAXIQUE_H

// Déclaration de la fonction principale qui correspond au programme entier
void Program();      // Démarre et contrôle la structure du programme

// Déclare le bloc principal ou les blocs imbriqués (corps de programme)
void Bloc();         // Gère le début et la fin des blocs

// Déclare l'ensemble des instructions (la séquence d'instructions à exécuter)
void Insts();        // Lit et analyse plusieurs instructions en séquence

// Déclare une instruction unique
void Inst();         // Analyse une instruction simple

// Déclare la condition d'une structure conditionnelle ou d'une boucle
void Cond();         // Vérifie et traite une condition (if, while, etc.)

// Déclare une expression arithmétique ou booléenne
void Exp();          // Analyse une expression complète

// Déclare un terme dans une expression (souvent une partie entre '+' et '-')
void Term();         // Gère les multiplications ou divisions dans une expression

// Déclare un facteur dans une expression (nombre, variable, expression entre parenthèses)
void Fact();         // Analyse l'unité la plus simple d'une expression

// Déclare l'analyse d'une instruction 'repeat...until'
void RepeatInst();   // Traite le bloc d'instructions qui se répète jusqu'à une condition

// Déclare l'analyse d'une instruction 'for'
void ForInst();      // Gère la boucle for avec ses paramètres et son corps

// Déclare l'analyse d'une instruction 'case'
void CaseInst();     // Analyse le choix multiple (case/of)

// -------------------------------
// Parties concernant les procédures et fonctions
// -------------------------------

// Déclare la partie où sont définies les procédures et fonctions
void ProcFuncPart(); // Traite la partie du code où les procédures et fonctions sont déclarées

// Déclaration de la procédure (procédure sans retour de valeur)
void ProcDecl();     // Analyse la déclaration d'une procédure

// Déclaration de la fonction (fonction avec retour de valeur)
void FuncDecl();     // Analyse la déclaration d'une fonction

// Analyse un appel de fonction ou une affectation
void CallOrAssign(); // Gère l'appel à une fonction ou l'affectation à une variable

// Variable statique indiquant si on est dans une fonction ou non
// 0 = pas dans une fonction, 1 = dans une fonction
static int insideAFunction = 0;      

// Variable statique pour stocker le nom de la fonction courante
static char currentFunctionName[64]; // Nom de la fonction que l'on analyse actuellement

#endif
