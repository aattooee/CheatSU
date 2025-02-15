#include "linux/types.h"
#define GET_MODULE_BASE_MAX_MODULE_NAME_LEN 100+1
#define _COPY_MEMORY_MAX_OFFSETS_LEN 10
typedef struct _COPY_MEMORY
{
	pid_t pid;
	uintptr_t addr;
	void *buffer;
	size_t size;
	size_t offsets_count;
	uintptr_t offsets[_COPY_MEMORY_MAX_OFFSETS_LEN];
} COPY_MEMORY, *PCOPY_MEMORY;

typedef struct _MODULE_BASE
{
	pid_t pid;
	char *name;
	uintptr_t base;
} MODULE_BASE, *PMODULE_BASE;

enum OPERATIONS
{
	OP_INIT_KEY = 0x900,
	OP_READ_MEM = 0x901,
	OP_WRITE_MEM = 0x902,
	OP_MODULE_BASE = 0x903,
};
enum CHEAT_OP_RES{
	ERROR_OP_FAILED = -1,
	OP_SUCCESS = 0,
	ERROR_UNKNOWN_OP = 1,
	ERROR_ARGS_INVALID =2,
	ERROR_TOUCH_ARGS_FAILED =3,
	ERROR_READ_MEM_FAILED = 4,
	ERROR_WRITE_MEM_FAILED = 5,
	ERROR_GET_MODULE_BASE_FAILED =6
};
