# Introduction

This is **your worklog** when discovering programming for embedded systems. 
It should contain what you will need to reopen this project in 6 months 
or a year and be able to understand and evolve it.

Also, as an Appendix, there should be the necessary details about what is
new to you, something like a cheat sheet in some sense. Something you can 
go back to when your memory will not server you well.

# First Sprint

- [Setup](./setup.md)
- [Understanding the build](./build.md)
- [Understanding the execution](./execution.md)
- [Advanced debugging](./debugging.md)

Pour ce cours et au cours des manipulations, nous allons utliser un QEMU qui va suimuluer une bord. Nous n'aurons donc pas de bord physique. 

## 1 étape : Compréhension du Makefile   : make run
En tant normale la toolchain (compilateur, assembleur, linker) produit un binaire machine pour une architecture précise.Or QEMU peut émuler un processeur précis. il faudra donc veiller à ce que les archectures soient les mêmes.
Il faudra particulèrement veiller au type de processeur, l'architecture, l'adresse memoire, board, etc

## 2 étape : lancer en mode debuger   : make debug
Comme on utitlise un  similateur, il faut lancer deux terminales différents (un pour l'enrtée sortie de la borde simuler )et le deuxieme pour le mode debug. 


## 3 étape : jouer sur l’envoi et le retour des caractères.
Faire attention au fait que lorsque je clear la console de la board, c’est différent de clear ma propre console.

## 4 étape : le timer

Comment marche un timer ?
Le timer est une partie de la board qui va compter à la fréquence de l’horloge. On peut le configurer pour compter le temps, par exemple.

Comme le timer suit la cadence de l’horloge, il faudra convertir la fréquence en temps. La fréquence étant très élevée, il faut normalement diminuer la fréquence de l’horloge si possible, sinon utiliser un prescaler, qui divisera la fréquence.

Pour configurer le timer, il faut regarder où il est situé (quel périphérique) et également choisir quel timer on va utiliser. Il existe plusieurs types de timers (ils ont chacun des fonctionnalités différentes : interruptions, etc.).

Ne pas oublier la fréquence finale pour la convertir en secondes, par exemple.