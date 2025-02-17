#include "memory.h"
#include "asm/page-def.h"
#include "linux/stddef.h"
#include "linux/types.h"
#include <linux/tty.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#include <asm/cpu.h>
#include <asm/io.h>
#include <asm/page.h>
#include <asm/pgtable.h>

extern struct mm_struct *get_task_mm(struct task_struct *task);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 61))
extern void mmput(struct mm_struct *);

phys_addr_t translate_linear_address(struct mm_struct *mm, uintptr_t va)
{
	pgd_t *pgd;
	p4d_t *p4d;
	pmd_t *pmd;
	pte_t *pte;
	pud_t *pud;

	phys_addr_t page_addr;
	uintptr_t page_offset;

	pgd = pgd_offset(mm, va);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		return 0;
	}
	p4d = p4d_offset(pgd, va);
	if (p4d_none(*p4d) || p4d_bad(*p4d)) {
		return 0;
	}
	pud = pud_offset(p4d, va);
	if (pud_none(*pud) || pud_bad(*pud)) {
		return 0;
	}
	pmd = pmd_offset(pud, va);
	if (pmd_none(*pmd)) {
		return 0;
	}
	pte = pte_offset_kernel(pmd, va);
	if (pte_none(*pte)) {
		return 0;
	}
	if (!pte_present(*pte)) {
		return 0;
	}
	// 页物理地址
	page_addr = (phys_addr_t)(pte_pfn(*pte) << PAGE_SHIFT);
	// 页内偏移
	page_offset = va & (PAGE_SIZE - 1);

	return page_addr + page_offset;
}
#else
phys_addr_t translate_linear_address(struct mm_struct *mm, uintptr_t va)
{
	pgd_t *pgd;
	pmd_t *pmd;
	pte_t *pte;
	pud_t *pud;

	phys_addr_t page_addr;
	uintptr_t page_offset;

	pgd = pgd_offset(mm, va);
	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		return 0;
	}
	pud = pud_offset(pgd, va);
	if (pud_none(*pud) || pud_bad(*pud)) {
		return 0;
	}
	pmd = pmd_offset(pud, va);
	if (pmd_none(*pmd)) {
		return 0;
	}
	pte = pte_offset_kernel(pmd, va);
	if (pte_none(*pte)) {
		return 0;
	}
	if (!pte_present(*pte)) {
		return 0;
	}
	// 页物理地址
	page_addr = (phys_addr_t)(pte_pfn(*pte) << PAGE_SHIFT);
	// 页内偏移
	page_offset = va & (PAGE_SIZE - 1);

	return page_addr + page_offset;
}
#endif

#ifndef ARCH_HAS_VALID_PHYS_ADDR_RANGE
static inline int valid_phys_addr_range(phys_addr_t addr, size_t count)
{
	return addr + count <= __pa(high_memory);
}
#endif

bool read_physical_address(phys_addr_t pa, void *buffer, size_t size,
			   bool to_kernel)
{
	void *mapped;

	if (!pfn_valid(__phys_to_pfn(pa))) {
		return false;
	}
	if (!valid_phys_addr_range(pa, size)) {
		return false;
	}
	mapped = ioremap_cache(pa, size);
	if (!mapped) {
		return false;
	}
	if (to_kernel) {
		if (copy_from_kernel_nofault(buffer, mapped, size)) {
			iounmap(mapped);
			return false;
		}
		iounmap(mapped);
		return true;
	}
	if (copy_to_user(buffer, mapped, size)) {
		iounmap(mapped);
		return false;
	}
	iounmap(mapped);
	return true;
}

bool write_physical_address(phys_addr_t pa, void *buffer, size_t size)
{
	void *mapped;

	if (!pfn_valid(__phys_to_pfn(pa))) {
		return false;
	}
	if (!valid_phys_addr_range(pa, size)) {
		return false;
	}
	mapped = ioremap_cache(pa, size);
	if (!mapped) {
		return false;
	}
	if (copy_from_user(mapped, buffer, size)) {
		iounmap(mapped);
		return false;
	}
	iounmap(mapped);
	return true;
}

bool read_process_memory(pid_t pid, uintptr_t addr, void *buffer, size_t size,
			 size_t offsets_count, uintptr_t *offsets)
{
	struct task_struct *task;
	struct mm_struct *mm;
	struct pid *pid_struct;
	phys_addr_t pa;
	int current_offset_idx = 0;
	size_t bytes_could_read = 0;
	uintptr_t page_offset;
	pid_struct = find_get_pid(pid);
	if (!pid_struct) {
		return false;
	}
	task = get_pid_task(pid_struct, PIDTYPE_PID);
	if (!task) {
		return false;
	}
	mm = get_task_mm(task);
	if (!mm) {
		return false;
	}

	while (current_offset_idx + 1 < offsets_count) {
		addr += offsets[current_offset_idx];
		pa = translate_linear_address(mm, addr);
		if (!pa) {
			goto failed;
		}
		if (!read_physical_address(pa, &addr, sizeof(addr), true)) {
			goto failed;
		}
		current_offset_idx++;
	}
	if (offsets_count != 0)
		addr += offsets[current_offset_idx];
	pa = translate_linear_address(mm, addr);
	if (!pa) {
		goto failed;
	}

	//跨页读取:
	page_offset = addr & (PAGE_SIZE - 1);
	bytes_could_read = size>PAGE_SIZE - page_offset?PAGE_SIZE - page_offset:size;
	while (true) {
		if (read_physical_address(pa, buffer, bytes_could_read,
					  false)) {
			size -= bytes_could_read;
			if (size == 0)
				goto success;
			addr += bytes_could_read;
			buffer += bytes_could_read;

			pa = translate_linear_address(mm, addr);
			if (!pa)
				goto failed;
			bytes_could_read = size >= PAGE_SIZE ?
						   PAGE_SIZE :
						   size;
		} else
			goto failed;
	}
success:
	mmput(mm);
	return true;
failed:
	mmput(mm);
	return false;
}

bool write_process_memory(pid_t pid, uintptr_t addr, void *buffer, size_t size)
{
	struct task_struct *task;
	struct mm_struct *mm;
	struct pid *pid_struct;
	phys_addr_t pa;

	pid_struct = find_get_pid(pid);
	if (!pid_struct) {
		return false;
	}
	task = get_pid_task(pid_struct, PIDTYPE_PID);
	if (!task) {
		return false;
	}
	mm = get_task_mm(task);
	if (!mm) {
		return false;
	}
	mmput(mm);
	pa = translate_linear_address(mm, addr);
	if (!pa) {
		return false;
	}
	return write_physical_address(pa, buffer, size);
}
