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

## 5 étape : Les interruptions 

Objectif : ne plus faire du polling sur l’UART et réagir uniquement quand une touche est pressée.

Vue d’ensemble du flux : 
-> Taper une touche 
-> Le UART0 (PL011) reçoit un octet et lève une IRQ RX.
-> Le VIC (contrôleur d’interruptions) relaie cette IRQ au CPU.
-> Le CPU saute dans le vecteur d’IRQ (exception.s) et appelle le handler C.
-> Le handler C lit les octets, affiche (echo), puis nettoie l’interruption. (code dans le hanlder, code a excuté)

Graphique pour un meilleur visuel :

            Interruption matérielle
                    ↓
            CPU entre en mode IRQ
                    ↓
            exception.s (assembleur)
                    ↓
            isr_handler()
                    ↓
            Lecture registre VIC
                    ↓
            Détection bit actif
                    ↓
            Appel irq_table[i].callback()



mis en place :
- Activation du contrôleur d’interruptions (VIC) et configuration d’une IRQ pour l’UART0.
- Activation de l’interruption RX dans le PL011 (UART0) pour être notifié dès qu’un caractère arrive.
- Gestionnaire d’IRQ côté assembleur qui sauvegarde/restaure les registres et appelle un handler C.
- Handler C qui lit tous les octets disponibles et les ré-affiche (echo), puis nettoie l’interruption RX.
- Boucle principale remplacée par `wfi()` pour mettre le CPU en attente et le réveiller par IRQ.

Fichiers clés :
- `irq.S` : fonctions bas niveau `_irqs_setup`, `_irqs_enable`, `_irqs_disable`, `_wfi`.
- `exception.s` : vecteur d’exception + retour propre de l’IRQ.
- `isr.c` : table des callbacks et dispatch des IRQ du VIC.
- `main.c` : init UART0 IRQ + handler d’echo.
- `versatile.ld` : ajout d’un petit stack IRQ (`irq_stack_top`).
- `Makefile` : ajout de `irq.o` et `isr.o`.

Notes / pièges :
- Sans linker `irq.S`, les symboles `_irqs_*` et `_wfi` sont introuvables.
- Il faut un stack IRQ dédié, sinon crash en entrée d’IRQ.
- Après traitement, bien effacer le flag d’interruption RX (UART ICR), sinon IRQ plus jamais répétées car toujours levé. 