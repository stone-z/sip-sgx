/* Micro Snake, based on a simple simple snake game by Simon Huggins
 *
 * Copyright (c) 2003, 2004  Simon Huggins <webmaster@simonhuggins.com>
 * Copyright (c) 2009  Joachim Nilsson <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Original Borland Builder C/C++ snake code available at Simon's home page
 * http://www.simonhuggins.com/courses/cbasics/course_notes/snake.htm
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

#include "Enclave_u.h"
#include "sgx_urts.h"
#include "sgx_utils/sgx_utils.h"
#include "conio.h"
#include "snake.h"

#ifdef DEBUG
#define DBG(fmt, args...) fprintf (stderr, fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

sgx_enclave_id_t global_eid = 0;

unsigned int usec_delay = DEFAULT_DELAY;

void ocall_printf(const char* str) {
   printf("%s\n", str);
}

void ocall_print_char(char character) {
   printf("%c\n", character);
}

void ocall_print_int(const char* text, int num) {
   printf(text, num);
}

void ocall_print_str(const char* text, const char* val) {
  printf(text, val);
}
void ocall_clrscr(const char* str) {
   puts(str);
}

void ocall_gotoxy(int x, int y) {
   printf("\e[%d;%dH", y, x);
}

void ocall_textattr(int attr) {
   textattr(attr);
}

void ocall_textcolor(int color) {
   textcolor(color);
}

void ocall_textbackground(int color) {
   textbackground(color);
}

int ocall_rand() {
   return rand();
}

int sigsetup (int signo, void (*callback)(int))
{
   struct sigaction action;

   sigemptyset(&action.sa_mask);
   //sigaddset(&action.sa_mask, signo);
   action.sa_flags = 0;
   action.sa_handler = callback;
   if (SIGALRM == signo)
   {
#ifdef SA_INTERRUPT
      action.sa_flags |= SA_INTERRUPT; /* SunOS 4.x */
#endif
   }
   else
   {
#ifdef SA_RESTART
      action.sa_flags |= SA_RESTART; /* SVR4, 4.4BSD */
#endif
   }

   return sigaction (signo, &action, NULL);
}

void sig_handler (int signal __attribute__ ((unused)))
{
   clrscr ();
   DBG("Received signal %d\n", signal);
   int status = system("stty sane");
   exit(WEXITSTATUS(status));
}

void alarm_handler (int signal __attribute__ ((unused)))
{
   static struct itimerval val;

   if (!signal)
   {
      sigsetup (SIGALRM, alarm_handler);
   }

   val.it_value.tv_sec  = 0;
   val.it_value.tv_usec = usec_delay;

   setitimer (ITIMER_REAL, &val, NULL);
}

void ocall_draw_line (int col, int row)
{
   int i;

   gotoxy (col, row);
   textbackground (LIGHTBLUE);
   textcolor (LIGHTBLUE);

   for (i = 0; i < MAXCOL + 2; i++)
   {
      if (i == 0 || i == MAXCOL + 1)
         printf ("+");
      else
         printf ("-");
   }

   textattr (RESETATTR);
}

void move (snake_t *snake, char keys[], char key)
{
   int i;
   direction_t prev = snake->dir;

   if (key == keys[RIGHT])
   {
      snake->dir = RIGHT;
   }
   else if (key == keys[LEFT])
   {
      snake->dir = LEFT;
   }
   else if (key == keys[UP])
   {
      snake->dir = UP;
   }
   else if (key == keys[DOWN])
   {
      snake->dir = DOWN;
   }
   else if (key == keys[LEFT_TURN])
   {
      switch (prev)
      {
         case LEFT:
            snake->dir = DOWN;
            break;

         case RIGHT:
            snake->dir = UP;
            break;

         case UP:
            snake->dir = LEFT;
            break;

         case DOWN:
            snake->dir = RIGHT;
            break;

         default:
            break;
      }
   }
   else if (key == keys[RIGHT_TURN])
   {
      switch (prev)
      {
         case LEFT:
            snake->dir = UP;
            break;

         case RIGHT:
            snake->dir = DOWN;
            break;

         case UP:
            snake->dir = RIGHT;
            break;

         case DOWN:
            snake->dir = LEFT;
            break;

         default:
            break;
      }
   }

   switch (snake->dir)
   {
      case LEFT:
         snake->body[snake->len].row = snake->body[snake->len - 1].row;
         snake->body[snake->len].col = snake->body[snake->len - 1].col - 1;
         break;

      case RIGHT:
         snake->body[snake->len].row = snake->body[snake->len - 1].row;
         snake->body[snake->len].col = snake->body[snake->len - 1].col + 1;
         break;

      case UP:
         snake->body[snake->len].row = snake->body[snake->len - 1].row - 1;
         snake->body[snake->len].col = snake->body[snake->len - 1].col;
         break;

      case DOWN:
         snake->body[snake->len].row = snake->body[snake->len - 1].row + 1;
         snake->body[snake->len].col = snake->body[snake->len - 1].col;
         break;

      default:
         /* NOP */
         break;
   }

   /* Blank last segment of snake */
   textattr (RESETATTR);
   gotoxy (snake->body[0].col + 1, snake->body[0].row + 1);
   puts (" ");

   /* ... and remove it from the array */
   for (i = 1; i <= snake->len; i++)
   {
      snake->body[i - 1] = snake->body[i];
   }

   /* Display snake in yellow */
   textbackground (YELLOW);
   for (i = 0; i < snake->len; i++)
   {
      gotoxy (snake->body[i].col + 1, snake->body[i].row + 1);
      puts (" ");
   }
   textattr (RESETATTR);
#ifdef DEBUG
   gotoxy (71, 1);
   printf ("(%02d,%02d)", snake->body[snake->len - 1].col, snake->body[snake->len - 1].row);
#endif
}

int collide_walls (snake_t *snake)
{
   snake_segment_t *head = &snake->body[snake->len - 1];

   if ((head->row > MAXROW) || (head->row < 1) ||
       (head->col > MAXCOL) || (head->col < 1))
   {
      DBG("Wall collision.\n");
      return 1;
   }

   return 0;
}

int collide_object (snake_t *snake, screen_t *screen, char object)
{
   snake_segment_t *head = &snake->body[snake->len - 1];

   if (screen->grid[head->row - 1][head->col - 1] == object)
   {
      DBG("Object '%c' collision.\n", object);
      return 1;
   }

   return 0;
}

int collide_self (snake_t *snake)
{
   int i;
   snake_segment_t *head = &snake->body[snake->len - 1];

   for (i = 0; i < snake->len - 1; i++)
   {
      snake_segment_t *body = &snake->body[i];

      if (head->row == body->row && head->col == body->col)
      {
         DBG("Self collision.\n");
         return 1;
      }
   }

   return 0;
}

int collision (snake_t *snake, screen_t *screen)
{
   return collide_walls (snake) ||
      collide_object (snake, screen, CACTUS) ||
      collide_self (snake);
}

int eat_gold (snake_t *snake, screen_t *screen)
{
   snake_segment_t *head = &snake->body[snake->len - 1];

   /* We're called after collide_object() so we know it's
    * a piece of gold at this position.  Eat it up! */
   screen->grid[head->row - 1][head->col - 1] = ' ';

   screen->gold--;
   screen->score += snake->len * screen->obstacles;
   snake->len++;

   if (screen->score > screen->high_score)
   {
      screen->high_score = screen->score; /* New high score! */
   }

   return screen->gold;
}

void output(sgx_status_t status) {
   std::cout << "Status " << status << std::endl;
   if (status != SGX_SUCCESS) {
      std::cout << "unsuccessful" << std::endl;
   }
}


int main (void)
{
   
   srand(time(NULL));

   if (initialize_enclave(&global_eid, "enclave.token", "enclave.signed.so") < 0) {
      std::cout << "Fail to initialize enclave." << std::endl;
      return 1;
   }

   char keypress;
   snake_t snake;
   screen_t screen;
   char keys[NUM_KEYS] = DEFAULT_KEYS;

   int status = system ("stty cbreak -echo stop u");
   if(WEXITSTATUS(status))
   {
      fprintf (stderr, "Failed setting up the screen, is 'stty' missing?\n");
      return 1;
   }

   /* Call it once to initialize the timer. */
   alarm_handler (0);

   sigsetup (SIGINT, sig_handler);
   sigsetup (SIGHUP, sig_handler);
   sigsetup (SIGTERM, sig_handler);

   do
   {
      sgx_status_t status = setup_level (global_eid, &screen, &snake, 1);
      output(status);    
 
      do
      {
         keypress = (char)getchar ();

         /* Move the snake one position. */
         move (&snake, keys, keypress);

         /* keeps cursor flashing in one place instead of following snake */
         gotoxy (1, 1);

         if (collision (&snake, &screen))
         {
            keypress = keys[QUIT];
            break;
         }
         else if (collide_object (&snake, &screen, GOLD))
         {
            /* If no gold left after consuming this one... */
            if (!eat_gold (&snake, &screen))
            {
               /* ... then go to next level. */
               status = setup_level (global_eid, &screen, &snake, 0);
               output(status);
            }

            status = show_score (global_eid, &screen);
            output(status);
         }
      }
      while (keypress != keys[QUIT]);

      status = show_score (global_eid, &screen);
      output(status);

      gotoxy (32, 6);
      textcolor (LIGHTRED);
      printf ("-G A M E  O V E R-");

      gotoxy (32, 9);
      textcolor (YELLOW);
      printf ("Another Game (y/n)? ");

      do
      {
         keypress = getchar ();
      }
      while ((keypress != 'y') && (keypress != 'n'));
   }
   while (keypress == 'y');

   clrscr ();

   int status2 = system ("stty sane");
   return WEXITSTATUS(status2);
}


/**
 * Local Variables:
 *  version-control: t
 *  c-file-style: "ellemtel"
 * End:
 */
