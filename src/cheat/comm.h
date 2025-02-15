#include "linux/types.h"
typedef struct _COPY_MEMORY
{
	pid_t pid;
	uintptr_t addr;
	void *buffer;
	size_t size;
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