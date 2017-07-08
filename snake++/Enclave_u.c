#include "Enclave_u.h"
#include <errno.h>

typedef struct ms_setup_level_t {
	screen_t* ms_screen;
	snake_t* ms_snake;
	int ms_level;
} ms_setup_level_t;

typedef struct ms_show_score_t {
	screen_t* ms_screen;
} ms_show_score_t;

typedef struct ms_collision_t {
	int ms_retval;
	snake_t* ms_snake;
	screen_t* ms_screen;
} ms_collision_t;

typedef struct ms_collide_object_t {
	int ms_retval;
	snake_t* ms_snake;
	screen_t* ms_screen;
	char ms_object;
} ms_collide_object_t;

typedef struct ms_eat_gold_t {
	int ms_retval;
	snake_t* ms_snake;
	screen_t* ms_screen;
} ms_eat_gold_t;


typedef struct ms_ocall_printf_t {
	char* ms_str;
} ms_ocall_printf_t;

typedef struct ms_ocall_clrscr_t {
	char* ms_str;
} ms_ocall_clrscr_t;

typedef struct ms_ocall_draw_line_t {
	int ms_col;
	int ms_row;
} ms_ocall_draw_line_t;

typedef struct ms_ocall_DBG_t {
	char* ms_string;
	char ms_object;
} ms_ocall_DBG_t;

typedef struct ms_ocall_collide_walls_t {
	int ms_retval;
	snake_t* ms_snake;
} ms_ocall_collide_walls_t;

typedef struct ms_ocall_collide_self_t {
	int ms_retval;
	snake_t* ms_snake;
} ms_ocall_collide_self_t;

typedef struct ms_ocall_move_t {
	snake_t* ms_snake;
	char ms_key;
} ms_ocall_move_t;

typedef struct ms_ocall_getchar_t {
	int ms_retval;
} ms_ocall_getchar_t;

typedef struct ms_GetTimeInMillis_t {
	uint64 ms_retval;
} ms_GetTimeInMillis_t;

static sgx_status_t SGX_CDECL Enclave_ocall_printf(void* pms)
{
	ms_ocall_printf_t* ms = SGX_CAST(ms_ocall_printf_t*, pms);
	ocall_printf((const char*)ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_clrscr(void* pms)
{
	ms_ocall_clrscr_t* ms = SGX_CAST(ms_ocall_clrscr_t*, pms);
	ocall_clrscr((const char*)ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_draw_line(void* pms)
{
	ms_ocall_draw_line_t* ms = SGX_CAST(ms_ocall_draw_line_t*, pms);
	ocall_draw_line(ms->ms_col, ms->ms_row);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_DBG(void* pms)
{
	ms_ocall_DBG_t* ms = SGX_CAST(ms_ocall_DBG_t*, pms);
	ocall_DBG((const char*)ms->ms_string, ms->ms_object);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_collide_walls(void* pms)
{
	ms_ocall_collide_walls_t* ms = SGX_CAST(ms_ocall_collide_walls_t*, pms);
	ms->ms_retval = ocall_collide_walls(ms->ms_snake);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_collide_self(void* pms)
{
	ms_ocall_collide_self_t* ms = SGX_CAST(ms_ocall_collide_self_t*, pms);
	ms->ms_retval = ocall_collide_self(ms->ms_snake);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_move(void* pms)
{
	ms_ocall_move_t* ms = SGX_CAST(ms_ocall_move_t*, pms);
	ocall_move(ms->ms_snake, ms->ms_key);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_ocall_getchar(void* pms)
{
	ms_ocall_getchar_t* ms = SGX_CAST(ms_ocall_getchar_t*, pms);
	ms->ms_retval = ocall_getchar();

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL Enclave_GetTimeInMillis(void* pms)
{
	ms_GetTimeInMillis_t* ms = SGX_CAST(ms_GetTimeInMillis_t*, pms);
	ms->ms_retval = GetTimeInMillis();

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[9];
} ocall_table_Enclave = {
	9,
	{
		(void*)Enclave_ocall_printf,
		(void*)Enclave_ocall_clrscr,
		(void*)Enclave_ocall_draw_line,
		(void*)Enclave_ocall_DBG,
		(void*)Enclave_ocall_collide_walls,
		(void*)Enclave_ocall_collide_self,
		(void*)Enclave_ocall_move,
		(void*)Enclave_ocall_getchar,
		(void*)Enclave_GetTimeInMillis,
	}
};
sgx_status_t setup_level(sgx_enclave_id_t eid, screen_t* screen, snake_t* snake, int level)
{
	sgx_status_t status;
	ms_setup_level_t ms;
	ms.ms_screen = screen;
	ms.ms_snake = snake;
	ms.ms_level = level;
	status = sgx_ecall(eid, 0, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t show_score(sgx_enclave_id_t eid, screen_t* screen)
{
	sgx_status_t status;
	ms_show_score_t ms;
	ms.ms_screen = screen;
	status = sgx_ecall(eid, 1, &ocall_table_Enclave, &ms);
	return status;
}

sgx_status_t collision(sgx_enclave_id_t eid, int* retval, snake_t* snake, screen_t* screen)
{
	sgx_status_t status;
	ms_collision_t ms;
	ms.ms_snake = snake;
	ms.ms_screen = screen;
	status = sgx_ecall(eid, 2, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t collide_object(sgx_enclave_id_t eid, int* retval, snake_t* snake, screen_t* screen, char object)
{
	sgx_status_t status;
	ms_collide_object_t ms;
	ms.ms_snake = snake;
	ms.ms_screen = screen;
	ms.ms_object = object;
	status = sgx_ecall(eid, 3, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t eat_gold(sgx_enclave_id_t eid, int* retval, snake_t* snake, screen_t* screen)
{
	sgx_status_t status;
	ms_eat_gold_t ms;
	ms.ms_snake = snake;
	ms.ms_screen = screen;
	status = sgx_ecall(eid, 4, &ocall_table_Enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t do_game(sgx_enclave_id_t eid)
{
	sgx_status_t status;
	status = sgx_ecall(eid, 5, &ocall_table_Enclave, NULL);
	return status;
}

