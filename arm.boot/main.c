#include "main.h"
#include "uart.h"
#include "event.h"
#include "timer.h"
#include "isr.h"
#include "ring.h"
#include "terminal_funct.h"
#include "function_main.h"

// je sais pas trop si ca sert encore mais je touche pas
#define ECHO_ZZZ

int secondes = 0;
int event_count = 0;  // nb d'events dans la queue
int total_events = 0; // ca compte tout, meme les vieux
static struct event *ready_head = NULL;
char input_line[80]; // 80 ca devrait suffire non?
uint8_t input_offset = 0;

static void rx_bottom_handler(void *cookie);
static void blink_bottom_handler(void *cookie);

// les timers sont en memoire mappee, jsp trop pourquoi static
static timer_regs_t *timer1 = (timer_regs_t *)TIMER1_BASE;
static volatile bool_t cursor_is_visible = FALSE; // volatile obligatoire sinon ca marche pas (vu en cours)
static struct event rx_event = {.cookie = NULL, .react = rx_bottom_handler, .eta = 0, .next = NULL, .posted = FALSE};
static struct event blink_event = {.cookie = NULL, .react = blink_bottom_handler, .eta = 0, .next = NULL, .posted = FALSE};

// on attend le prochain event en dormant (low power truc)
static void sleep_until_next_event(void)
{
  core_disable_interrupts();
  if (ready_head == NULL) // si ya rien a faire on dort
  {
    wfi(); // wait for interrupt, vu dans le cours slide 42
  }
  core_enable_interrupts(); // TODO: verifier si ca pose pas de race condition
}

void shell(char *cmd_line, uint8_t cmd_len)
{
  // commande clear
  if (cmd_len == 5 &&
      cmd_line[0] == 'c' &&
      cmd_line[1] == 'l' &&
      cmd_line[2] == 'e' &&
      cmd_line[3] == 'a' &&
      cmd_line[4] == 'r')
  {
    clear_screen(UART0);
    display_status(UART0, secondes, total_events);
    print_prompt(UART0);
  }
  // commande quit
  else if (cmd_len == 4 &&
           cmd_line[0] == 'q' &&
           cmd_line[1] == 'u' &&
           cmd_line[2] == 'i' &&
           cmd_line[3] == 't')
  {
    uart_send_string(UART0, "\r\nBye! A la prochaine!\r\n"); // message de fin stylé
    qemu_exit();
  }
  // commande echo (faut que ca fasse plus de 5 chars sinon ya rien apres le espace)
  else if (cmd_len > 5 &&
           cmd_line[0] == 'e' &&
           cmd_line[1] == 'c' &&
           cmd_line[2] == 'h' &&
           cmd_line[3] == 'o' &&
           cmd_line[4] == ' ')
  {
    uart_send_string(UART0, "\r\nEchoing: ");
    uart_send_string(UART0, cmd_line + 5); // +5 pour sauter "echo "
    print_prompt(UART0);
  }
  else // commande inconnue, on fait rien mais on reaffiche le prompt
  {
    print_prompt(UART0);
  }
}

// handler pour la reception uart (la partie "bottom" du truc)
static void rx_bottom_handler(void *cookie)
{
  (void)cookie; // on utilise pas cookie mais faut le mettre pour le compilo
  process_ring(input_line, &input_offset);
}

// handler pour le clignotement du curseur, appele toutes les 500ms
static void blink_bottom_handler(void *cookie)
{
  (void)cookie;
  if (cursor_is_visible) // si visible -> cacher + incrementer secondes
  {
    cursor_hide(UART0);
    cursor_is_visible = FALSE;
    secondes += 1; // une seconde de plus (toutes les 2x 500ms)
    display_status(UART0, secondes, total_events);
  }
  else // sinon -> montrer
  {
    cursor_show(UART0);
    cursor_is_visible = TRUE;
  }
}

// interruption uart0, appelée quand un caractere arrive
static void uart0_irq_handler(uint32_t irq, void *cookie)
{
  (void)irq;
  (void)cookie;
  uint8_t c;

  // on lit tout ce qui est dans le buffer
  uint8_t code = uart_receive(UART0, &c);
  while (code)
  {
    ring_put(c); // on met dans le ring buffer
    code = uart_receive(UART0, &c);
  }

  mmio_write32(UART0, UART_ICR, UART_RXIC); // clear l'interruption, IMPORTANT sinon boucle infinie
  event_post_front(&rx_event, &event_count, &total_events, &ready_head);
}

static void uart0_irq_init(void)
{
  mmio_write32(UART0, UART_ICR, 0x7FF); // on clear tout les interruptions en attente
  mmio_set(UART0, UART_IMSC, UART_RXIM); // active uniquement RX (on a pas besoin du reste)
  irq_enable(UART0_IRQ, uart0_irq_handler, NULL);
}

// interruption du timer1, toutes les 500ms
static void timer1_irq_handler(uint32_t irq, void *cookie)
{
  (void)irq;
  (void)cookie;

  timer1->IntClr = 1; // acquitter l'interruption (sinon ca revient tout de suite)
  event_post_front(&blink_event, &event_count, &total_events, &ready_head);
}

static void timer1_irq_init(void)
{
  // timer a 1MHz donc 500000 ticks = 0.5 secondes
  // j'ai verifie avec la calculette
  timer1->Control = 0x00; // reset
  timer1->Load = 500000;
  timer1->BGLoad = 500000; // jsp la diff entre Load et BGLoad mais les deux
  timer1->IntClr = 1;
  timer1->Control = 0xE2; // Enable=1, Periodic=1, IntEnable=1, 32bit (cf datasheet)

  irq_enable(TIMER1_IRQ, timer1_irq_handler, NULL);
}

// point d'entree C, apres l'init en asm dans startup.s
void _start()
{
  // affichage du message de bienvenue
  clear_screen(UART0);
  uart_send_string(UART0, "\n");
  uart_send_string(UART0, "\nFor information:\n");
  uart_send_string(UART0, "  - Quit with \"C-a c\" to get to the QEMU console.\n");
  uart_send_string(UART0, "  - Then type in \"quit\" to stop QEMU.\n");
  uart_send_string(UART0, "  - type in my console   \"quit\" \n");
  uart_send_string(UART0, "\n -- My console -- \n");

  timer_init(); // init les timers (dans timer.c)

  // setup des interruptions
  irqs_setup();
  uart0_irq_init();
  timer1_irq_init();
  core_enable_interrupts(); // APRES le setup sinon ca crash

  cursor_hide(UART0);
  cursor_is_visible = FALSE;
  display_status(UART0, secondes, total_events);
  print_prompt(UART0);

  // boucle principale, event-driven
  while (1) // boucle infinie
  {
    struct event *evt = event_pop(&event_count, &ready_head);
    if (evt != NULL)
    {
      evt->react(evt->cookie); // on execute le handler de l'event
    }
    else
    {
      sleep_until_next_event(); // pas d'event -> on dort et on attend de se reveiller avec une interruption (timer ou uart)
    }
  }
}
