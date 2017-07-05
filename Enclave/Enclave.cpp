#include <stdio.h>
#include "sgx_trts.h"
#include "Enclave_t.h"

/* Default 0.2 sec between snake movement. */
unsigned int usec_delay = DEFAULT_DELAY;

/* If level==0 then just move on to the next level
 * if level==1 restart game
 * Otherwise start game at that level. */
void setup_level (screen_t *screen, snake_t *snake, int level)
{
   int i, row, col;

   //srand ((unsigned int)time (NULL));

   /* Initialize on (re)start */
   if (1 == level)
   {
      screen->score = 0;
      screen->obstacles = 4;
      screen->level = 1;
      snake->speed = 14;
      snake->dir = RIGHT;
   }
   else
   {
      screen->score += screen->level * 1000;
      screen->obstacles += 2;
      screen->level++;          /* add to obstacles */

      if ((screen->level % 5 == 0) && (snake->speed > 1))
      {
         snake->speed--;        /* increase snake->speed every 5 levels */
      }
   }

   /* Set up global variables for new level */
   screen->gold = 0;
   snake->len = level + 4;
   usec_delay = DEFAULT_DELAY - level * 10000;

   /* Fill grid with blanks */
   for (row = 0; row < MAXROW; row++)
   {
      for (col = 0; col < MAXCOL; col++)
      {
         screen->grid[row][col] = ' ';
      }
   }
 
   /* Fill grid with objects */
   for (i = 0; i < screen->obstacles * 2; i++)
   {
      /* Find free space to place an object on. */
      do
      {
         //unsigned char row_buf[4];
         //unsigned char col_buf[4];
         //sgx_read_rand(row_buf, 4);
         //sgx_read_rand(col_buf, 4);
         //row = *((int*)row_buf) % MAXROW;
         //col = *((int*)col_buf) % MAXCOL;
         int* row_buf = new int[1]; 
         int* col_buf = new int[1];
         ocall_rand(row_buf);
         row  = *row_buf % MAXROW;
         ocall_rand(col_buf);
         col = *col_buf % MAXCOL;
      }
      while (screen->grid[row][col] != ' ');

      if (i < screen->obstacles)
      {
         screen->grid[row][col] = CACTUS;
      }
      else
      {
         screen->gold++;
         screen->grid[row][col] = GOLD;
      }
   }

   /* Create snake array of length snake->len */
   for (i = 0; i < snake->len; i++)
   {
      snake->body[i].row = START_ROW;
      snake->body[i].col = snake->dir == LEFT ? START_COL - i : START_COL + i;
   }

   /* Draw playing board */
   //clrscr();
   ocall_clrscr("\e[2J\e[1;1H");

   ocall_draw_line (1, 1);

   for (row = 0; row < MAXROW; row++)
   {
      ocall_gotoxy(1, row + 2);

      ocall_textcolor (LIGHTBLUE);
      ocall_textbackground (LIGHTBLUE);
      ocall_printf ("|");
      ocall_textattr (RESETATTR);

      ocall_textcolor (WHITE);
      for (col = 0; col < MAXCOL; col++)
      {
         ocall_print_char(screen->grid[row][col]);
      }

      ocall_textcolor (LIGHTBLUE);
      ocall_textbackground (LIGHTBLUE);
      ocall_printf ("|");
      ocall_textattr (RESETATTR);
   }

   ocall_draw_line (1, MAXROW + 2);

   show_score (screen);

   ocall_textcolor (LIGHTRED);
   //gotoxy (3, 1);
   //printf ("h:Help");
   ocall_gotoxy (30, 1);
   ocall_print_str ("[ Micro Snake v%s ]", VERSION);
}

void show_score (screen_t *screen)
{
   ocall_textcolor (LIGHTCYAN);
   ocall_gotoxy (3, MAXROW + 2);
   ocall_print_int ("Level: %05d", screen->level);

   ocall_textcolor (YELLOW);
   ocall_gotoxy (21, MAXROW + 2);
   ocall_print_int ("Gold Left: %05d", screen->gold);

   ocall_textcolor (LIGHTGREEN);
   ocall_gotoxy (43, MAXROW + 2);
   ocall_print_int ("Score: %05d", screen->score);

   ocall_textcolor (LIGHTMAGENTA);
   ocall_gotoxy (61, MAXROW + 2);
   ocall_print_int ("High Score: %05d", screen->high_score);
}



