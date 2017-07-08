#include "Enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */

#include <errno.h>
#include <string.h> /* for memcpy etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)


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

static sgx_status_t SGX_CDECL sgx_setup_level(void* pms)
{
	ms_setup_level_t* ms = SGX_CAST(ms_setup_level_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	screen_t* _tmp_screen = ms->ms_screen;
	size_t _len_screen = sizeof(*_tmp_screen);
	screen_t* _in_screen = NULL;
	snake_t* _tmp_snake = ms->ms_snake;
	size_t _len_snake = sizeof(*_tmp_snake);
	snake_t* _in_snake = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_setup_level_t));
	CHECK_UNIQUE_POINTER(_tmp_screen, _len_screen);
	CHECK_UNIQUE_POINTER(_tmp_snake, _len_snake);

	if (_tmp_screen != NULL) {
		_in_screen = (screen_t*)malloc(_len_screen);
		if (_in_screen == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_screen, _tmp_screen, _len_screen);
	}
	if (_tmp_snake != NULL) {
		_in_snake = (snake_t*)malloc(_len_snake);
		if (_in_snake == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_snake, _tmp_snake, _len_snake);
	}
	setup_level(_in_screen, _in_snake, ms->ms_level);
err:
	if (_in_screen) {
		memcpy(_tmp_screen, _in_screen, _len_screen);
		free(_in_screen);
	}
	if (_in_snake) {
		memcpy(_tmp_snake, _in_snake, _len_snake);
		free(_in_snake);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_show_score(void* pms)
{
	ms_show_score_t* ms = SGX_CAST(ms_show_score_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	screen_t* _tmp_screen = ms->ms_screen;
	size_t _len_screen = sizeof(*_tmp_screen);
	screen_t* _in_screen = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_show_score_t));
	CHECK_UNIQUE_POINTER(_tmp_screen, _len_screen);

	if (_tmp_screen != NULL) {
		_in_screen = (screen_t*)malloc(_len_screen);
		if (_in_screen == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_screen, _tmp_screen, _len_screen);
	}
	show_score(_in_screen);
err:
	if (_in_screen) {
		memcpy(_tmp_screen, _in_screen, _len_screen);
		free(_in_screen);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_collision(void* pms)
{
	ms_collision_t* ms = SGX_CAST(ms_collision_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	snake_t* _tmp_snake = ms->ms_snake;
	size_t _len_snake = sizeof(*_tmp_snake);
	snake_t* _in_snake = NULL;
	screen_t* _tmp_screen = ms->ms_screen;
	size_t _len_screen = sizeof(*_tmp_screen);
	screen_t* _in_screen = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_collision_t));
	CHECK_UNIQUE_POINTER(_tmp_snake, _len_snake);
	CHECK_UNIQUE_POINTER(_tmp_screen, _len_screen);

	if (_tmp_snake != NULL) {
		_in_snake = (snake_t*)malloc(_len_snake);
		if (_in_snake == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_snake, _tmp_snake, _len_snake);
	}
	if (_tmp_screen != NULL) {
		_in_screen = (screen_t*)malloc(_len_screen);
		if (_in_screen == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_screen, _tmp_screen, _len_screen);
	}
	ms->ms_retval = collision(_in_snake, _in_screen);
err:
	if (_in_snake) {
		memcpy(_tmp_snake, _in_snake, _len_snake);
		free(_in_snake);
	}
	if (_in_screen) {
		memcpy(_tmp_screen, _in_screen, _len_screen);
		free(_in_screen);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_collide_object(void* pms)
{
	ms_collide_object_t* ms = SGX_CAST(ms_collide_object_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	snake_t* _tmp_snake = ms->ms_snake;
	size_t _len_snake = sizeof(*_tmp_snake);
	snake_t* _in_snake = NULL;
	screen_t* _tmp_screen = ms->ms_screen;
	size_t _len_screen = sizeof(*_tmp_screen);
	screen_t* _in_screen = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_collide_object_t));
	CHECK_UNIQUE_POINTER(_tmp_snake, _len_snake);
	CHECK_UNIQUE_POINTER(_tmp_screen, _len_screen);

	if (_tmp_snake != NULL) {
		_in_snake = (snake_t*)malloc(_len_snake);
		if (_in_snake == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_snake, _tmp_snake, _len_snake);
	}
	if (_tmp_screen != NULL) {
		_in_screen = (screen_t*)malloc(_len_screen);
		if (_in_screen == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_screen, _tmp_screen, _len_screen);
	}
	ms->ms_retval = collide_object(_in_snake, _in_screen, ms->ms_object);
err:
	if (_in_snake) {
		memcpy(_tmp_snake, _in_snake, _len_snake);
		free(_in_snake);
	}
	if (_in_screen) {
		memcpy(_tmp_screen, _in_screen, _len_screen);
		free(_in_screen);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_eat_gold(void* pms)
{
	ms_eat_gold_t* ms = SGX_CAST(ms_eat_gold_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	snake_t* _tmp_snake = ms->ms_snake;
	size_t _len_snake = sizeof(*_tmp_snake);
	snake_t* _in_snake = NULL;
	screen_t* _tmp_screen = ms->ms_screen;
	size_t _len_screen = sizeof(*_tmp_screen);
	screen_t* _in_screen = NULL;

	CHECK_REF_POINTER(pms, sizeof(ms_eat_gold_t));
	CHECK_UNIQUE_POINTER(_tmp_snake, _len_snake);
	CHECK_UNIQUE_POINTER(_tmp_screen, _len_screen);

	if (_tmp_snake != NULL) {
		_in_snake = (snake_t*)malloc(_len_snake);
		if (_in_snake == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_snake, _tmp_snake, _len_snake);
	}
	if (_tmp_screen != NULL) {
		_in_screen = (screen_t*)malloc(_len_screen);
		if (_in_screen == NULL) {
			status = SGX_ERROR_OUT_OF_MEMORY;
			goto err;
		}

		memcpy(_in_screen, _tmp_screen, _len_screen);
	}
	ms->ms_retval = eat_gold(_in_snake, _in_screen);
err:
	if (_in_snake) {
		memcpy(_tmp_snake, _in_snake, _len_snake);
		free(_in_snake);
	}
	if (_in_screen) {
		memcpy(_tmp_screen, _in_screen, _len_screen);
		free(_in_screen);
	}

	return status;
}

static sgx_status_t SGX_CDECL sgx_do_game(void* pms)
{
	sgx_status_t status = SGX_SUCCESS;
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	do_game();
	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv;} ecall_table[6];
} g_ecall_table = {
	6,
	{
		{(void*)(uintptr_t)sgx_setup_level, 0},
		{(void*)(uintptr_t)sgx_show_score, 0},
		{(void*)(uintptr_t)sgx_collision, 0},
		{(void*)(uintptr_t)sgx_collide_object, 0},
		{(void*)(uintptr_t)sgx_eat_gold, 0},
		{(void*)(uintptr_t)sgx_do_game, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[9][6];
} g_dyn_entry_table = {
	9,
	{
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
		{0, 0, 0, 0, 0, 0, },
	}
};


sgx_status_t SGX_CDECL ocall_printf(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_printf_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_printf_t);
	void *__tmp = NULL;

	ocalloc_size += (str != NULL && sgx_is_within_enclave(str, _len_str)) ? _len_str : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_printf_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_printf_t));

	if (str != NULL && sgx_is_within_enclave(str, _len_str)) {
		ms->ms_str = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_str);
		memcpy((void*)ms->ms_str, str, _len_str);
	} else if (str == NULL) {
		ms->ms_str = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(0, ms);


	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_clrscr(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_clrscr_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_clrscr_t);
	void *__tmp = NULL;

	ocalloc_size += (str != NULL && sgx_is_within_enclave(str, _len_str)) ? _len_str : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_clrscr_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_clrscr_t));

	if (str != NULL && sgx_is_within_enclave(str, _len_str)) {
		ms->ms_str = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_str);
		memcpy((void*)ms->ms_str, str, _len_str);
	} else if (str == NULL) {
		ms->ms_str = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(1, ms);


	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_draw_line(int col, int row)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_draw_line_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_draw_line_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_draw_line_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_draw_line_t));

	ms->ms_col = col;
	ms->ms_row = row;
	status = sgx_ocall(2, ms);


	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_DBG(const char* string, char object)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_string = string ? strlen(string) + 1 : 0;

	ms_ocall_DBG_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_DBG_t);
	void *__tmp = NULL;

	ocalloc_size += (string != NULL && sgx_is_within_enclave(string, _len_string)) ? _len_string : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_DBG_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_DBG_t));

	if (string != NULL && sgx_is_within_enclave(string, _len_string)) {
		ms->ms_string = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_string);
		memcpy((void*)ms->ms_string, string, _len_string);
	} else if (string == NULL) {
		ms->ms_string = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_object = object;
	status = sgx_ocall(3, ms);


	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_collide_walls(int* retval, snake_t* snake)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_snake = sizeof(*snake);

	ms_ocall_collide_walls_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_collide_walls_t);
	void *__tmp = NULL;

	ocalloc_size += (snake != NULL && sgx_is_within_enclave(snake, _len_snake)) ? _len_snake : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_collide_walls_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_collide_walls_t));

	if (snake != NULL && sgx_is_within_enclave(snake, _len_snake)) {
		ms->ms_snake = (snake_t*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_snake);
		memcpy(ms->ms_snake, snake, _len_snake);
	} else if (snake == NULL) {
		ms->ms_snake = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(4, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_collide_self(int* retval, snake_t* snake)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_snake = sizeof(*snake);

	ms_ocall_collide_self_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_collide_self_t);
	void *__tmp = NULL;

	ocalloc_size += (snake != NULL && sgx_is_within_enclave(snake, _len_snake)) ? _len_snake : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_collide_self_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_collide_self_t));

	if (snake != NULL && sgx_is_within_enclave(snake, _len_snake)) {
		ms->ms_snake = (snake_t*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_snake);
		memcpy(ms->ms_snake, snake, _len_snake);
	} else if (snake == NULL) {
		ms->ms_snake = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(5, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_move(snake_t* snake, char key)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_snake = sizeof(*snake);

	ms_ocall_move_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_move_t);
	void *__tmp = NULL;

	ocalloc_size += (snake != NULL && sgx_is_within_enclave(snake, _len_snake)) ? _len_snake : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_move_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_move_t));

	if (snake != NULL && sgx_is_within_enclave(snake, _len_snake)) {
		ms->ms_snake = (snake_t*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_snake);
		memcpy(ms->ms_snake, snake, _len_snake);
	} else if (snake == NULL) {
		ms->ms_snake = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_key = key;
	status = sgx_ocall(6, ms);

	if (snake) memcpy((void*)snake, ms->ms_snake, _len_snake);

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_getchar(int* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_ocall_getchar_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_getchar_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_getchar_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_getchar_t));

	status = sgx_ocall(7, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL GetTimeInMillis(uint64* retval)
{
	sgx_status_t status = SGX_SUCCESS;

	ms_GetTimeInMillis_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_GetTimeInMillis_t);
	void *__tmp = NULL;


	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_GetTimeInMillis_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_GetTimeInMillis_t));

	status = sgx_ocall(8, ms);

	if (retval) *retval = ms->ms_retval;

	sgx_ocfree();
	return status;
}

