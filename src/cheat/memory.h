#include <linux/kernel.h>
#include <linux/sched.h>

phys_addr_t translate_linear_address(struct mm_struct *mm, uintptr_t va);

bool read_physical_address(phys_addr_t pa, void *buffer, size_t size,bool to_kernel);

bool write_physical_address(phys_addr_t pa, void *buffer, size_t size);

bool read_process_memory(pid_t pid, uintptr_t addr, void *buffer, size_t size,
			 size_t offsets_count, uintptr_t *offsets);

bool write_process_memory(pid_t pid, uintptr_t addr, void *buffer, size_t size);
