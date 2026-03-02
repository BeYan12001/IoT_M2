# ARM Boot - Projet IoT M2

Simulation d'un système embarqué ARM sur QEMU (board `versatilepb`).

## Prérequis

- `qemu-system-arm`
- `arm-none-eabi-gcc` (toolchain ARM)
- `make`

## Build & Run

```bash
# Compiler
make
# Lancer
make run
# Déboguer
make debug
```

> Pour quitter QEMU : `Ctrl+a c` puis `quit`, ou taper `quit` dans la console de la board.

## Commandes shell disponibles

| Commande       | Description                  |
|----------------|------------------------------|
| `clear`        | Efface l'écran               |
| `echo <texte>` | Affiche le texte             |
| `quit`         | Quitte QEMU proprement       |

## Architecture

```
main.c          — boucle principale, shell, événements
uart.c          — driver UART PL011
isr.c / irq.S  — gestion des interruptions (VIC)
timer.c         — configuration du timer
ring.c          — ring buffer (ISR ↔ main loop)
event.c         — système top-half / bottom-half
terminal_funct.c — fonctions d'affichage ANSI
```

## Fonctionnement

- Le CPU dort (`wfi()`) entre deux événements
- Une IRQ UART ou Timer réveille le CPU
- Le handler IRQ (top-half) poste un événement
- La boucle principale exécute le bottom-half

## Limitations

- `secondes` est un `int` → crash après un certain temp
- `clear` QEMU ≠ `clear` terminal local
- Pas d'historique de commandes
- `eta` (événements temporisés) non exploité


## Worklog
Le fichier worklog à la racine du projet
- worklog.md