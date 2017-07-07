#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_satus_t etc. */

#include "snake.h"
#include "conio.h"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_printf, (const char* str));
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_clrscr, (const char* str));
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_draw_line, (int col, int row));
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_DBG, (const char* string, char object));
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_collide_walls, (snake_t* snake));
int SGX_UBRIDGE(SGX_NOCONVENTION, ocall_collide_self, (snake_t* snake));

sgx_status_t setup_level(sgx_enclave_id_t eid, screen_t* screen, snake_t* snake, int level);
sgx_status_t show_score(sgx_enclave_id_t eid, screen_t* screen);
sgx_status_t collision(sgx_enclave_id_t eid, int* retval, snake_t* snake, screen_t* screen);
sgx_status_t collide_object(sgx_enclave_id_t eid, int* retval, snake_t* snake, screen_t* screen, char object);
sgx_status_t eat_gold(sgx_enclave_id_t eid, int* retval, snake_t* snake, screen_t* screen);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
