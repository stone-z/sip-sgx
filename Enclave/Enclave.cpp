#include <stdio.h>
#include "sgx_trts.h"
#include "Enclave_t.h"

/* Default 0.2 sec between snake movement. */
unsigned int usec_delay = DEFAULT_DELAY;

void printf(const char* fmt, ...) {
   char buf[BUFSIZ] = {'\0'};
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(buf, BUFSIZ, fmt, ap);
   va_end(ap);
   ocall_printf(buf);
}

int collide_object (snake_t *snake, screen_t *screen, char object)
{
   snake_segment_t *head = &snake->body[snake->len - 1];

   if (screen->grid[head->row - 1][head->col - 1] == object)
   {
      ocall_DBG("Object '%c' collision.\n", object);
      return 1;
   }

   return 0;
}

int collision (snake_t *snake, screen_t *screen)
{
   int col_wal_ret;
   ocall_collide_walls(&col_wal_ret, snake);

   int col_self_ret;
   ocall_collide_self(&col_self_ret, snake);

   return col_wal_ret ||
      collide_object (snake, screen, CACTUS) ||
      col_self_ret;
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
         unsigned char row_buf[4];
         unsigned char col_buf[4];
         sgx_read_rand(row_buf, 4);
         sgx_read_rand(col_buf, 4);
         row = *((int*)row_buf) % MAXROW;
         col = *((int*)col_buf) % MAXCOL;
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
      gotoxy(1, row + 2);

      textcolor (LIGHTBLUE);
      textbackground (LIGHTBLUE);
      printf ("|");
      textattr (RESETATTR);

      textcolor (WHITE);
      for (col = 0; col < MAXCOL; col++)
      {
         printf("%c", screen->grid[row][col]);
      }

      textcolor (LIGHTBLUE);
      textbackground (LIGHTBLUE);
      printf ("|");
      textattr (RESETATTR);
   }

   ocall_draw_line (1, MAXROW + 2);

   show_score (screen);

   textcolor (LIGHTRED);
   gotoxy (30, 1);
   printf("[ Micro Snake v%s ]", VERSION);
}

void show_score (screen_t *screen)
{
   textcolor (LIGHTCYAN);
   gotoxy (3, MAXROW + 2);
   printf("Level: %05d", screen->level);

   textcolor (YELLOW);
   gotoxy (21, MAXROW + 2);
   printf("Gold Left: %05d", screen->gold);

   textcolor (LIGHTGREEN);
   gotoxy (43, MAXROW + 2);
   printf("Score: %05d", screen->score);

   textcolor (LIGHTMAGENTA);
   gotoxy (61, MAXROW + 2);
   printf("High Score: %05d", screen->high_score);
}


void do_game()
{
   char keypress;
   snake_t snake;
   screen_t screen;
   char keys[NUM_KEYS] = DEFAULT_KEYS;

   do
   {
      setup_level (&screen, &snake, 1);
      do
      {
         int res;
         ocall_getchar(&res);
         keypress = (char)res;

        // keypress = (char)getchar ();

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
         //keypress = getchar ();
         ocall_getchar(&res);
         keypress = (char)res; 
      }
      while ((keypress != 'y') && (keypress != 'n'));
   }
   while (keypress == 'y');

}
