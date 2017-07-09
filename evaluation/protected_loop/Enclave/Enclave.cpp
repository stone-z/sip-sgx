#include <stdio.h>
#include "sgx_trts.h"
#include "Enclave_t.h"

/* Default 0.2 sec between snake movement. */
unsigned int usec_delay = DEFAULT_DELAY;

uint64 setup_sum = 0;
int setup_count = 0;

uint64 showscore_sum = 0;
int showscore_count = 0;

uint64 collision_sum = 0;
int collision_count = 0;

uint64 cobj_sum = 0;
int cobj_count = 0;

uint64 eat_sum = 0;
int eat_count = 0;

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

	struct timeval tv;
	struct timeval tv2;
	gettimeofday(&tv, NULL);	
	int co = collide_object (snake, screen, CACTUS);
	gettimeofday(&tv2, NULL);	
	auto d = ((tv2.tv_sec - tv.tv_sec)*1000000L + tv2.tv_usec) - tv.tv_usec;
	cobj_sum += d;
	cobj_count++;


   int col_self_ret;
   ocall_collide_self(&col_self_ret, snake);

   return col_wal_ret ||
      co ||
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
	if (screen->score > screen->high_score){
		screen->high_score = screen->score;
	}
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

   	struct timeval tv;
	struct timeval tv2;
	gettimeofday(&tv, NULL);	
    	show_score (screen);
	gettimeofday(&tv2, NULL);	
	auto d = ((tv2.tv_sec - tv.tv_sec)*1000000L + tv2.tv_usec) - tv.tv_usec;
	showscore_sum += d;
	showscore_count++;

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
   screen.high_score = 0;
   char keys[NUM_KEYS] = DEFAULT_KEYS;

   do
   {
      	struct timeval tv;
	struct timeval tv2;
	gettimeofday(&tv, NULL);	
	setup_level (&screen, &snake, 1);
	gettimeofday(&tv2, NULL);	
	auto d = ((tv2.tv_sec - tv.tv_sec)*1000000L + tv2.tv_usec) - tv.tv_usec;
	setup_sum += d;
	setup_count++;
      do
      {
         int res;
         ocall_getchar(&res);
         keypress = (char)res;

         /* Move the snake one position. */
         ocall_move (&snake, keypress);

         /* keeps cursor flashing in one place instead of following snake */
         gotoxy (1, 1);


		struct timeval tvc, tvc2;
		gettimeofday(&tvc, NULL);	
		int c = collision (&snake, &screen);
		gettimeofday(&tvc2, NULL);	
		auto dc = ((tvc2.tv_sec - tvc.tv_sec)*1000000L + tvc2.tv_usec) - tvc.tv_usec;
		collision_sum += dc;
		collision_count++;

		struct timeval tvco, tvco2;
		gettimeofday(&tvco, NULL);	
		int co = collide_object (&snake, &screen, GOLD);
		gettimeofday(&tvco2, NULL);	
		auto dco = ((tvco2.tv_sec - tvco.tv_sec)*1000000L + tvco2.tv_usec) - tvco.tv_usec;
		cobj_sum += dco;
		cobj_count++;

         if (c)
         {
            keypress = keys[QUIT];
            break;
         }
         else if (co)
         {
            /* If no gold left after consuming this one... */

		struct timeval tveg, tveg2;
		gettimeofday(&tveg, NULL);	
		int eg = eat_gold (&snake, &screen);
		gettimeofday(&tveg2, NULL);	
		auto deg = ((tveg2.tv_sec - tveg.tv_sec)*1000000L + tveg2.tv_usec) - tveg.tv_usec;
		eat_sum += deg;
		eat_count++;

            if (!eg)
            {
               /* ... then go to next level. */
		struct timeval tv;
		struct timeval tv2;
		gettimeofday(&tv, NULL);	
		setup_level (&screen, &snake, 0);
		gettimeofday(&tv2, NULL);	
		auto d = ((tv2.tv_sec - tv.tv_sec)*1000000L + tv2.tv_usec) - tv.tv_usec;
		setup_sum += d;
		setup_count++;
            }
		struct timeval tv;
		struct timeval tv2;
		gettimeofday(&tv, NULL);	
            	show_score (&screen);
		gettimeofday(&tv2, NULL);	
		auto d = ((tv2.tv_sec - tv.tv_sec)*1000000L + tv2.tv_usec) - tv.tv_usec;
		showscore_sum += d;
		showscore_count++;

         }
      }
      while (keypress != keys[QUIT]);

      	struct timeval tvss;
	struct timeval tvss2;
	gettimeofday(&tvss, NULL);	
    	show_score (&screen);
	gettimeofday(&tvss2, NULL);	
	auto dss = ((tvss2.tv_sec - tvss.tv_sec)*1000000L + tvss2.tv_usec) - tvss.tv_usec;
	showscore_sum += dss;
	showscore_count++;

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

	printf("\n");
        printf("Setup Sum: %llu\n", setup_sum);
        printf("Setup Count: %i\n", setup_count);
	printf("Setup Average: %llu\n", (setup_sum/setup_count));
	printf("\n");
        printf("ShowScore Sum: %llu\n", showscore_sum);
        printf("ShowScore Count: %i\n", showscore_count);
	printf("ShowScore Average: %llu\n", (showscore_sum/showscore_count));
	printf("\n");
	printf("Collision Average: %llu\n", (collision_sum/collision_count));
	printf("EatGold Average: %llu\n", (eat_sum/eat_count));
	printf("CollideObject Average: %llu\n", (cobj_sum/cobj_count));
}
