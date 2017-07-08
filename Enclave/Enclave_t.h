#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */

#include "snake.h"
#include "conio.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif


void setup_level(screen_t* screen, snake_t* snake, int level);
void show_score(screen_t* screen);
int collision(snake_t* snake, screen_t* screen);
int collide_object(snake_t* snake, screen_t* screen, char object);
int eat_gold(snake_t* snake, screen_t* screen);
void do_game();

sgx_status_t SGX_CDECL ocall_printf(const char* str);
sgx_status_t SGX_CDECL ocall_clrscr(const char* str);
sgx_status_t SGX_CDECL ocall_draw_line(int col, int row);
sgx_status_t SGX_CDECL ocall_DBG(const char* string, char object);
sgx_status_t SGX_CDECL ocall_collide_walls(int* retval, snake_t* snake);
sgx_status_t SGX_CDECL ocall_collide_self(int* retval, snake_t* snake);
sgx_status_t SGX_CDECL ocall_move(snake_t* snake, char key);
sgx_status_t SGX_CDECL ocall_getchar(int* retval);
sgx_status_t SGX_CDECL GetTimeInMillis(uint64* retval);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
