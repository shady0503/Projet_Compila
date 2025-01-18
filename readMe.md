# Documentation Détaillée du Compilateur/Interpréteur

Ce document explique étape par étape le fonctionnement du système de compilation et d'interprétation, en mettant l'accent sur les principaux mots-clés (tokens) mémorisés, le rôle de la pile, la gestion de la mémoire, et le flux global du programme.

## 1. Fonctionnement Global du Système

Le système se décompose en plusieurs grandes étapes :

1. **Lecture et Analyse Lexicale**  
   Le fichier source est lu caractère par caractère pour être découpé en unités lexicales appelées **tokens**.

2. **Analyse Syntaxique**  
   Les tokens sont analysés pour vérifier la structure grammaticale du programme. Le parseur construit une représentation interne (arbre syntaxique ou séquence d'instructions) qui respecte la grammaire du langage.

3. **Analyse Sémantique et Génération du P-code**  
   On vérifie que les déclarations et l'utilisation des identifiants sont cohérentes (types, déclarations, etc.) et on génère un code intermédiaire appelé **P-code**, qui est une suite d'instructions simplifiées.

4. **Interprétation du P-code**  
   L'interpréteur lit le P-code et l'exécute en utilisant une pile (stack) pour gérer temporairement les valeurs, les appels de fonctions, et les opérations arithmétiques.

---

## 2. Analyse Lexicale et Mots-Clés (Tokens)

### Rôle de l'Analyse Lexicale
- **LireCar()** : Lit un caractère depuis le fichier source.
- **SymSuiv()** : Analyse la séquence de caractères pour former le prochain **token**.

### Tokens Importants
Voici quelques mots-clés et symboles mémorisés :

- **`program`**  
  - **Rôle :** Indique le début d’un programme.  
  - **Exemple :**  
    ```pascal
    program MonProgramme;
    ```
- **`const`**  
  - **Rôle :** Déclare des constantes qui ne changent pas durant l'exécution.  
  - **Exemple :**  
    ```pascal
    const Taux = 5;
    ```
- **`type`**  
  - **Rôle :** Permet de créer des alias pour des types de base.  
  - **Exemple :**  
    ```pascal
    type Entier = integer;
    ```
- **`var`**  
  - **Rôle :** Déclare des variables qui pourront être utilisées dans le programme.  
  - **Exemple :**  
    ```pascal
    var a, b: integer;
    ```
- **`if`, `then`, `else`**  
  - **Rôle :** Permettent de réaliser des branchements conditionnels.  
  - **Exemple :**  
    ```pascal
    if a > 10 then
      writeln('Grand')
    else
      writeln('Petit');
    ```
- **Boucles :**  
  - **`while`/`do`**, **`repeat`/`until`**, **`for`/`to`**  
  - **Exemple (while) :**  
    ```pascal
    while a < 100 do
      a := a + 1;
    ```
- **Délimiteurs de blocs : `begin` et `end`**  
  - **Rôle :** Encapsulent un groupe d’instructions.  
  - **Exemple :**  
    ```pascal
    begin
      writeln('Début');
      writeln('Fin');
    end;
    ```
- **Opérateurs**  
  - **Arithmétiques :** `+`, `-`, `*`, `/`  
  - **Affectation :** `:=`  
  - **Comparaison :** `=`, `<`, `>`, `<=`, `>=`, `<>`

Ces tokens sont identifiés et mémorisés pour permettre au parseur de reconnaître la structure du programme.

---

## 3. Analyse Syntaxique et Sémantique

### Analyse Syntaxique
- **Fonction Principale : `Program()`**  
  Cette fonction lance l'analyse de tout le programme. Elle s'assure que le programme commence par le mot-clé `program`, traite les déclarations (`const`, `type`, `var`), puis passe au bloc principal marqué par `begin` et `end`.

### Analyse Sémantique
- **Vérification des Déclarations** :  
  La table des symboles stocke des informations sur chaque identifiant (nom, type, adresse).  
  - **Exemple :** Si une variable `a` est déclarée comme un `integer`, elle sera stockée avec son adresse en mémoire et son type sera `TYPE_INT`.
- **Génération du P-code** :  
  En analysant chaque instruction, le compilateur génère une série d'instructions P-code (comme `LDI`, `ADD`, `CALL`, etc.) qui représenteront les actions à réaliser lors de l'exécution.

---

## 4. La Pile (Stack) et Son Importance

### Rôle Fondamental de la Pile
La **pile** est une structure de données essentielle pour :
- **Stocker temporairement des valeurs** pendant l'exécution (résultats d'opérations, variables intermédiaires).
- **Gérer les appels de fonctions/procédures** en sauvegardant l'adresse de retour, les paramètres, et le pointeur de base (BP).

### Comment Fonctionne la Pile ?
- **Stack Pointer (SP)**  
  - **Description :** Indique la position actuelle du sommet de la pile.  
  - **Fonctionnement :**  
    - **Pousser** une valeur augmente SP et la nouvelle valeur est stockée à `MEM[SP]`.
    - **Dépiler** une valeur décrémente SP.
  - **Initialisation :** SP est initialisé à `-1` pour indiquer qu'elle est vide.

- **Base Pointer (BP)**  
  - **Description :** Utilisé lors des appels de fonctions pour référencer la base de la pile du contexte actuel.
  - **Fonctionnement lors d'un appel :**  
    1. Sauvegarder l'ancienne valeur de BP.
    2. Pousser sur la pile l'adresse de retour (l'instruction suivante) et l'ancien BP.
    3. Mettre à jour BP pour pointer vers la nouvelle base (c'est-à-dire la zone où les paramètres et autres informations locales seront stockés).

### Exemple Pratique
Imaginons une fonction qui prend deux paramètres et retourne leur somme :

- **Avant l'appel :**
  - La pile contient éventuellement quelques valeurs.
  - Les paramètres sont empilés sur la pile.

- **Lors de l'appel (`CALL`) :**
  - Le nombre de paramètres est déposé sur la pile.
  - L'adresse de retour et l'ancien BP sont sauvegardés.
  - BP est mis à jour.
  - Les arguments sont copiés dans la zone locale, accessible via BP (souvent à partir de `BP + 2`).

- **Retour de la fonction (`RET`) :**
  - Le résultat est placé en haut de la pile.
  - Les paramètres et les informations locales sont dépilés.
  - BP est restauré, et le contrôle revient à l'adresse de retour.

---

## 5. Interprétation du P-code

### Rôle de l'Interpréteur
L'interpréteur exécute le P-code généré par l'analyse syntaxique et sémantique. Son fonctionnement se base sur :
- La lecture d'une instruction par tour de boucle.
- L'exécution de cette instruction en manipulant la pile et la mémoire.

### Les Instructions P-code
Chaque instruction possède :
- **Un mnémonique (MNE) :** Par exemple, `LDI` (load integer), `ADD` (addition), etc.
- **Un argument (SUITE) :** Qui peut être une valeur, une adresse, ou un compteur selon l'instruction.

### Flux d'Exécution Simple (Exemple)
Imaginons un petit P-code :


**Déroulement :**
- **Étape 0 :** `LDI 10` → SP passe de -1 à 0, et `MEM[0]` devient 10.
- **Étape 1 :** `LDI 20` → SP passe à 1, et `MEM[1]` devient 20.
- **Étape 2 :** `ADD` → Les valeurs 10 et 20 sont dépilées, leur somme 30 est poussée sur la pile.
- **Étape 3 :** `PRN` → La valeur 30 est affichée.
- **Étape 4 :** `HLT` → L'interpréteur termine l'exécution.

---

# Notes Complémentaires

## Implémentation des Tableaux et Enregistrements (Records)
La gestion complète des tableaux et des enregistrements (structures) n'est **pas implémentée** dans ce code.

---

## Procédures et Fonctions
L'implémentation des procédures et fonctions, incluant le passage d’arguments (en mode valeur et en mode adresse), est **partiellement stable**. Certaines parties fonctionnent correctement tandis que d'autres peuvent être sujettes à des problèmes. Pensez à tester et, si nécessaire, à améliorer cette partie selon vos besoins.

---

## Points Faibles et Évaluation Selon les Consignes
Le code a été évalué par rapport aux critères suivants :

### **Type réel (codé sur 4 octets) (1 point)**

- **Observation** :
  Le type réel est implémenté via `float`, qui est généralement un type sur 4 octets en C.
- **Conclusion** :
  ✔️ Respecté – Le type réel répond à la consigne.

---

### **Type Tableau et Record (enregistrement) (2 points)**

- **Observation** :
  Le code ne fournit pas d’implémentation pour les tableaux et les enregistrements.
- **Conclusion** :
  ❌ Non implémenté – Ce point n’est pas couvert et ne rapportera donc pas de points.

---

### **Le ELSE dans le IF avec des IF imbriqués (1 point)**

- **Observation** :
  La structure conditionnelle `IF ... THEN ... ELSE` est gérée, y compris l’imbrication de `IF` pour les `else`.
- **Conclusion** :
  ✔️ Respecté – Le code prend bien en compte le `ELSE` et les `IF` imbriqués.

---

### **La structure de contrôle FOR (1 point)**

- **Observation** :
  La structure `FOR` est implémentée dans la fonction `ForInst()` et gère les itérations avec `to` et `downto`.
- **Conclusion** :
  ✔️ Respecté – La structure `FOR` répond aux spécifications.

---

### **La structure CASE (1 point)**

- **Observation** :
  La structure `CASE` est implémentée dans la fonction `CaseInst()`. Elle traite les cas numériques et inclut éventuellement une branche `ELSE`.
- **Conclusion** :
  ✔️ Respecté – Le `CASE` est présent et fonctionne comme attendu.

---

### **La structure REPEAT (répéter) (1 point)**

- **Observation** :
  La structure `REPEAT ... UNTIL` est implémentée via la fonction `RepeatInst()`.
- **Conclusion** :
  ✔️ Respecté – La structure `REPEAT` est correctement implémentée.

---

### **Les Procédures et Fonctions**
- **Sans passage d’arguments (2 points)**
- **Avec passage des arguments en mode valeur et en mode adresse (4 points)**

- **Observation** :
  Le code dispose d’implémentations pour les procédures (`ProcDecl()`) et les fonctions (`FuncDecl()`).
  Le mécanisme prend en charge les appels sans arguments ainsi que ceux avec arguments, en gérant le passage par valeur et par adresse (passage par référence).  
  Toutefois, certaines parties de cette implémentation restent fragiles ou instables.
- **Conclusion** :
  ⚠️ Partiellement respecté – La base est en place mais la gestion des paramètres, en particulier pour le passage par référence, est sujette à caution. Cela pourrait entraîner une perte de points dans une évaluation stricte.

---

## Résumé Global
- **Type réel (4 octets)** : ✔️ (respecté)  
- **Tableaux et enregistrements (records)** : ❌ (non implémenté)  
- **IF/ELSE avec IF imbriqués** : ✔️  
- **Structure FOR** : ✔️  
- **Structure CASE** : ✔️  
- **Structure REPEAT** : ✔️  
- **Procédures et Fonctions (argument passing)** : ⚠️ Partiellement respecté (implémentation de base présente, mais avec quelques faiblesses)  

---

## Conclusion
En conclusion, le code traite correctement la plupart des structures de contrôle (`IF/ELSE`, `FOR`, `CASE`, `REPEAT`) et gère correctement le type réel. Cependant :
- Il **manque l’implémentation des tableaux et des records**.
- L’implémentation des **procédures et fonctions**, notamment pour la gestion complète des arguments, reste fragile.  

Ces points faibles doivent être améliorés pour obtenir une conformité totale avec les consignes données.  

N'hésitez pas à explorer et modifier le code si vous souhaitez renforcer ces aspects ou ajouter de nouvelles fonctionnalités.

# How to Run

To compile and run the program, open a terminal and use the following commands:

```bash
# Compile the program
gcc -o main.exe main.c analyse_lexical.c syntaxique.c semantique.c interpreteur.c generation_pcode.c

# Run the executable
./main.exe test_path pcodefile_path




good luck li 9ra ou wsl hna hhhhhhhhhhhhhhhhhhh
