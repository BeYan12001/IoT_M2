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

## 1 étape : Compréhension du Makefile : `make run`
La toolchain (compilateur, assembleur, linker) génère un binaire pour une architecture précise. QEMU émule un processeur tout aussi précis, donc les paramètres doivent correspondre.
Points à vérifier dans le Makefile : type de CPU, architecture, adresse mémoire de chargement, board/plateforme ciblée, et options QEMU associées.
PS: pour quitter Ctrl+a-c, puis "quit"  

## 2 étape : Lancer en mode débogueur : `make debug`
Avec un simulateur, il faut deux terminaux :
1) un terminal pour l’I/O de la board simulée (console série QEMU),
2) un terminal pour le débogage (GDB/serveur de debug).
Le bon déroulement dépend du bon couplage entre le port de debug ouvert par QEMU et la session GDB.
Dans le cas d'une vrai board, le QEMU simule l'entrée/sortie de cette board, en plus de ca on a egalement le terminale de la machine. 
PS: pour quitter Ctrl+a-c, puis "quit".
Le mode debug n'est pas graphique, pour manipuler voir **gdb.md**.

## 3 étape : Jouer sur l’envoi et le retour des caractères
Objectif : valider l’I/O UART (entrée utilisateur + echo).
Point d’attention : `clear` sur la console de la board (UART/QEMU) n’efface pas la console de mon terminal local, et inversement. Je n'ai pas eu le temps de corriger cela car ce n'est pas normal.


## 4 étape : le timer
Comment marche un timer ?
Le timer est une partie de la board qui va compter à la fréquence de l’horloge. On peut le configurer pour compter le temps, par exemple.

Comme le timer suit la cadence de l’horloge, il faudra convertir la fréquence en temps. La fréquence étant très élevée, il faut normalement diminuer la fréquence de l’horloge si possible, sinon utiliser un prescaler, qui divisera la fréquence.

Pour configurer le timer, il faut regarder où il est situé (quel périphérique) et également choisir quel timer on va utiliser. Il existe plusieurs types de timers (ils ont chacun des fonctionnalités différentes : interruptions, etc.).

Ne pas oublier la fréquence finale pour la convertir en secondes, par exemple.

### Pourquoi utiliser une interruption timer ?
Pour une action périodique (ex: clignotement du curseur), il y a deux approches :

1) **Polling dans la boucle `main`**  

2) **Interruption timer**  
On programme le timer pour lever une IRQ périodique (ex: toutes les 500 ms), puis on exécute l’action dans le handler IRQ.
- Avantage: déclenchement régulier et plus déterministe.
- Avantage: compatible avec `wfi()` (CPU dort entre événements, puis se réveille sur IRQ).
- Avantage: la boucle `main` reste simple (traitement applicatif), le timing périodique est géré par le matériel.

### Lien avec notre code
Dans mon implémentation, le clignotement du curseur est fait par IRQ timer :
- le timer est configuré en mode périodique,
- chaque interruption alterne `cursor_show(UART0)` et `cursor_hide(UART0)`,
- on efface le flag d’interruption (`IntClr`) dans le handler.

Pour des événements périodiques, l’interruption timer est plus propre et plus robuste que compter dans la boucle `main` avec des conditions. En effet, le Polling dans la boucle `main` vérifie tout le temps, même quand rien ne se passe (CPU occupé inutilement).






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
- on réserve proprement une pile IRQ alignée, add stack.
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


## - étape : Ring method
L’idée est: 
- Utiliser un buffer circulaire de taille fixe, afin d’éviter toute gestion dynamique de la mémoire.
- L’implémenter de manière à permettre une utilisation concurrente, tout en restant sans verrou (lock-free).

Buffer circulaire :
Il sert de canal de communication entre l’interruption (ISR) et la boucle principale.
Cette méthode permet d’éviter un buffer de taille variable et de prévenir les débordements (overflow).
