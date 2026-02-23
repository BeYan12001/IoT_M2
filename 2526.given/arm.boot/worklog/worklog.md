# Introduction

Pour ce cours et au cours des manipulations, je vais utliser un QEMU qui va simuler une board. Je n'ai donc pas de board physique. 

## 1 étape : Compréhension du Makefile : `make run`
La toolchain (compilateur, assembleur, linker) génère un binaire pour une architecture précise. QEMU émule un processeur tout aussi précis, donc les paramètres doivent correspondre.
Points à vérifier dans le Makefile : type de CPU, architecture, adresse mémoire de chargement, board/plateforme ciblée, et options QEMU associées.

PS: pour quitter Ctrl+a-c, puis "quit"  

Autre possibilité :
Dans le terminal de la board simulée, la commande quit fonctionne également.

Cela est rendu possible grâce à l’option -semihosting ajoutée dans les cibles run et debug du Makefile.Le semihosting permet au programme embarqué de dialoguer directement avec QEMU pour certaines opérations système.

Dans main.c, la fonction qemu_exit() utilise le mécanisme de semihosting ARM (appel système SYS_EXIT via svc 0x00123456) afin de demander à QEMU de s’arrêter proprement.

## 2 étape : Lancer en mode débogueur : `make debug`
Avec un simulateur, il faut deux terminaux :
1) un terminal pour l’I/O de la board simulée (console série QEMU),
2) un terminal pour le débogage (GDB/serveur de debug).
Le bon déroulement dépend du bon couplage entre le port de debug ouvert par QEMU et la session GDB.
Dans le cas d'une vrai board, le QEMU simule l'entrée/sortie de cette board, en plus de ca on a egalement le terminale de la machine. 
PS: pour quitter Ctrl+a-c, puis "quit".
Le mode debug n'est pas graphique, pour manipuler voir **gdb.md**.

## 3 étape : Jouer sur l’envoi et le retour des caractères
**Objectif :** 
valider l’I/O UART (entrée utilisateur + echo).
Point d’attention : `clear` sur la console de la board (UART/QEMU) n’efface pas la console de mon terminal local, et inversement. Je n'ai pas eu le temps de corriger cela car ce n'est pas normal.

## 4 étape : le timer
**Memo :** 
Un timer est un compteur automatique basé sur l’horloge du microcontrôleur qui permet de mesurer le temps ou de déclencher des actions à intervalles précis.

Comment marche un timer ? 
Le timer est une partie de la board qui va compter à la fréquence de l’horloge. On peut le configurer pour compter le temps, par exemple. Comme le timer suit la cadence de l’horloge, il faudra convertir la fréquence en temps. La fréquence étant très élevée, il faut normalement diminuer la fréquence de l’horloge si possible, sinon utiliser un prescaler, qui divisera la fréquence.

Pour configurer le timer, il faut regarder où il est situé (quel périphérique) et également choisir quel timer on va utiliser. Il existe plusieurs types de timers (ils ont chacun des fonctionnalités différentes : interruptions, etc.).

**Pourquoi utiliser une interruption timer ?**
Pour une action périodique (ex: clignotement du curseur), il y a deux approches :

1) **Polling dans la boucle `main`**
(Sollicite inutilement le processeur en vérifiant en permanence une condition en plus de compter, ce qui entraîne un gaspillage de ressources et une implémentation peu élégante.)

2) **Interruption timer**  
On programme le timer pour lever une IRQ périodique (ex: toutes les 500 ms), puis on exécute l’action dans le handler IRQ.
- Avantage: déclenchement régulier et plus déterministe.
- Avantage: compatible avec `wfi()` (CPU dort entre événements, puis se réveille sur IRQ).
- Avantage: la boucle `main` reste simple (traitement applicatif), le timing périodique est géré par le matériel.

**Lien avec notre code**
Dans mon implémentation, le clignotement du curseur est fait par IRQ timer :
- le timer est configuré en mode périodique,
- chaque interruption alterne `cursor_show(UART0)` et `cursor_hide(UART0)`,
- on efface le flag d’interruption (`IntClr`) dans le handler.

Pour des événements périodiques, l’interruption timer est plus propre et plus robuste que compter dans la boucle `main` avec des conditions. En effet, le Polling dans la boucle `main` vérifie tout le temps, même quand rien ne se passe.

**Notes / pièges :**
- Bien choisir la fréquence du timer et faire attention a la fréuquence de l'horloge pour calculer un temps coherent. 
- Si le timer génère des interruptions fréquentes, le programme principal n’a plus de temps d’exécution
- Ne pas oublier la fréquence finale pour la convertir en secondes, par exemple.

## 5 étape : Les interruptions 
**Memo:** 
Un mécanisme qui permet au microcontrôleur de réagir immédiatement à un événement important sans surveiller en permanence.En gros “Stop ce que tu fais, il se passe quelque chose d’important.”

**Objectif :** ne plus faire du polling sur l’UART et réagir uniquement quand une touche est pressée.

Vue d’ensemble du flux d'éxecution: 
-> Taper une touche 
-> Le UART0 (PL011) reçoit un octet et lève une IRQ RX.
-> Le VIC (contrôleur d’interruptions) relaie cette IRQ au CPU.
-> Le CPU saute dans le vecteur d’IRQ (exception.s) et appelle le handler C.
-> Le handler C lit les octets, affiche (echo), puis nettoie l’interruption. (code dans le hanlder, code a excuté)

Graphique :
```bash
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
```

**Mise en place :**
- Je réserve proprement une pile IRQ alignée, add stack.
- Activation du contrôleur d’interruptions (VIC) et configuration d’une IRQ pour l’UART0.
- Activation de l’interruption RX dans le PL011 (UART0) pour être notifié dès qu’un caractère arrive.
- Gestionnaire d’IRQ côté assembleur qui sauvegarde/restaure les registres et appelle un handler C.
- Handler C qui lit tous les octets disponibles et les ré-affiche (echo), puis nettoie l’interruption RX.
- Boucle principale remplacée par `wfi()` pour mettre le CPU en attente et le réveiller par IRQ.

**Notes / pièges :**
- Sans linker `irq.S`, les symboles `_irqs_*` et `_wfi` sont introuvables.
- Il faut un stack IRQ dédié, sinon crash en entrée d’IRQ.
- Après traitement, bien effacer le flag d’interruption RX (UART ICR), sinon IRQ plus jamais répétées car toujours levé.
- Veiller a toujours un handler court ( fonction qui est executé lors de la detection de l'interruption), le but est de faire des taches prioriatires et surtout COURTE. 

## 6 étape : Les événements (top-half / bottom-half)
**Objectif :** mixer interruptions et traitement applicatif sans alourdir les handlers IRQ.

Principe :
- **Top-half (IRQ)** : code très court, orienté matériel.
- **Bottom-half (event)** : réaction applicative exécutée dans la boucle principale.

Dans mon code :
- IRQ UART0 : lit les octets, les place dans le ring, nettoie le flag UART, puis poste `rx_event`.
- IRQ Timer1 : nettoie le flag timer, puis poste `blink_event`.
- Boucle principale : dépile un événement et exécute `evt->react(evt->cookie)`.

Structure utilisée :
```c
struct event {
  void* cookie;
  void (*react)(void* cookie);
  uint64_t eta;
  struct event* next;
  bool_t posted;
};
```

Rôle des champs :
- `react` : fonction à appeler quand l’événement est traité.
- `cookie` : contexte passé à `react`.
- `next` : chaînage dans la file.
- `posted` : évite de poster plusieurs fois le même event tant qu’il est déjà en attente.
- `eta` : prévu pour des événements temporisés (pas encore exploité ici).

Pourquoi c’est utile :
- handlers IRQ plus courts et plus sûrs.
- séparation claire entre matériel et logique métier.
- extensible : ajouter un nouveau périphérique = nouveau event + nouveau handler.
- moins de polling inutile, meilleur usage de `wfi()`.

**Notes / pièges :**
- Toujours acquitter l’IRQ dans le top-half.
- Un bottom-handler ne doit pas être posté en double (`posted`).


## - étape : Ring method
L’idée est: 
- Utiliser un buffer circulaire de taille fixe, afin d’éviter toute gestion dynamique de la mémoire.
- L’implémenter de manière à permettre une utilisation concurrente, tout en restant sans verrou (lock-free).

Buffer circulaire :
Il sert de canal de communication entre l’interruption (ISR) et la boucle principale.
Cette méthode permet d’éviter un buffer de taille variable et de prévenir les débordements (overflow).

