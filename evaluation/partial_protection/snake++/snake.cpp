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
#include <stdarg.h>

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

typedef long long int64; typedef unsigned long long uint64;

sgx_enclave_id_t global_eid = 0;

unsigned int usec_delay = DEFAULT_DELAY;

uint64 setup_sum = 0;
int setup_count = 0;

uint64 showscore_sum = 0;
int showscore_count = 0;

uint64 collision_sum = 0;
int collision_count = 0;

uint64 collide_sum = 0;
int collide_count = 0;

uint64 cobj_sum = 0;
int cobj_count = 0;

uint64 eat_sum = 0;
int eat_count = 0;

void ocall_printf(const char* str) {
   printf("%s", str);
}

void ocall_clrscr(const char* str) {
   puts(str);
}

void ocall_DBG(const char* str, char object) {
   DBG(str, object);
}

int ocall_getchar() {
   return getchar();
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

void ocall_move(snake_t* snake, char key) {
   char keys[NUM_KEYS] = DEFAULT_KEYS;
   move(snake, keys, key);
}

int ocall_collide_walls (snake_t *snake)
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


int ocall_collide_self (snake_t *snake)
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


void output(sgx_status_t status) {
   if (status != SGX_SUCCESS) {
      std::cout << "unsuccessful" << std::endl;
      exit(1);
   }
}

uint64 GetTimeInMillis(){
	// From https://stackoverflow.com/a/1861337
	struct timeval tv;

	gettimeofday(&tv, NULL);

	uint64 ret = tv.tv_usec;
	ret /= 1000;

	ret += (tv.tv_sec * 1000);

	return ret;
}

int main (void)
{
   if (initialize_enclave(&global_eid, "enclave.token", "enclave.signed.so") < 0) {
      std::cout << "Fail to initialize enclave." << std::endl;
      return 1;
   }

   uint64 t1 = GetTimeInMillis();

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
    
   char keypress;
   snake_t snake;
   screen_t screen;
   screen.high_score = 0;
   char keys[NUM_KEYS] = DEFAULT_KEYS;

   do
   {
      setup_level (&screen, &snake, 1);
      do
      {
         int res;
         ocall_getchar(&res);
         keypress = (char)res;

         /* Move the snake one position. */
         ocall_move (&snake, keypress);

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
            setup_level (&screen, &snake, 0);
         }

            show_score (&screen);
         }
      }
      while (keypress != keys[QUIT]);

      show_score (&screen);

      gotoxy (32, 6);
      textcolor (LIGHTRED);
      printf ("-G A M E  O V E R-");

      gotoxy (32, 9);
      textcolor (YELLOW);
      printf ("Another Game (y/n)? ");

      do
      {
         int res;
         ocall_getchar(&res);
         keypress = (char)res; 
      }
      while ((keypress != 'y') && (keypress != 'n'));
   }
   while (keypress == 'y');

   clrscr ();

   int status2 = system ("stty sane");
   return WEXITSTATUS(status2);
}
