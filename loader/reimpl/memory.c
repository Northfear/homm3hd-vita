#include <stdlib.h>
#include <string.h>
#include <psp2/types.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/clib.h>
#include "memory.h"

#define SCE_KERNEL_MAX_MAIN_CDIALOG_MEM_SIZE 0x8C6000
#define PHYCONT_THRESHHOLD 0
#define CDRAM_POOL_SIZE 0

typedef enum 
{
	MEM_PHYCONT,
	MEM_CDLG,
	MEM_CDRAM,
	MEM_NONE
} MemType;

static void *mempool_mspace[MEM_NONE] = { NULL, NULL, NULL };
static void *mempool_addr[MEM_NONE] = { NULL, NULL, NULL };
static SceUID mempool_id[MEM_NONE] = { 0, 0, 0 };
static size_t mempool_size[MEM_NONE] = { 0, 0, 0 };
static int mempool_init = 0;

void __real_free(void* ptr);
void *__real_malloc(size_t size);
void *__real_calloc(size_t num, size_t size);
void *__real_memalign(size_t alignment, size_t size);
void *__real_realloc(void *ptr, size_t size);


void *__wrap_memcpy(void *dest, const void *src, size_t n)
{
	return sceClibMemcpy(dest, src, n);
}

void *__wrap_memmove(void *dest, const void *src, size_t n)
{
	return sceClibMemmove(dest, src, n);
}

void *__wrap_memset(void *s, int c, size_t n)
{
	return sceClibMemset(s, c, n);
}

void __wrap_free(void* ptr)
{
	free_hook(ptr);
}

void *__wrap_malloc(size_t size)
{
	return malloc_hook(size);
}

void *__wrap_calloc(size_t num, size_t size)
{
	return calloc_hook(num, size);
}

void *__wrap_memalign(size_t alignment, size_t size)
{
	return memalign_hook(alignment, size);
}

void *__wrap_realloc(void *ptr, size_t size)
{
	return realloc_hook(ptr, size);
}


void init_memory()
{
	if (mempool_init)
		term_memory();

	SceKernelFreeMemorySizeInfo info;
	info.size = sizeof(SceKernelFreeMemorySizeInfo);
	sceKernelGetFreeMemorySize(&info);

#if GRAPHICS_API_VITAGL
	size_t phycont_size = 0;
#else
	size_t phycont_size = info.size_phycont > PHYCONT_THRESHHOLD ? info.size_phycont - PHYCONT_THRESHHOLD : 0;
#endif
	size_t cdlg_size = 0;
	size_t cdram_size = 0;

	mempool_size[MEM_PHYCONT] = ALIGN(phycont_size, 1024 * 1024);
	mempool_size[MEM_CDLG] = ALIGN(cdlg_size, 4 * 1024);
	mempool_size[MEM_CDRAM] = ALIGN(cdram_size, 256 * 1024);

	if (mempool_size[MEM_PHYCONT])
		mempool_id[MEM_PHYCONT] = sceKernelAllocMemBlock("phycont_mempool", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_RW, mempool_size[MEM_PHYCONT], NULL);

	if (mempool_size[MEM_CDLG])
		mempool_id[MEM_CDLG] = sceKernelAllocMemBlock("cdlg_mempool", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_CDIALOG_RW, mempool_size[MEM_CDLG], NULL);

	if (mempool_size[MEM_CDRAM])
		mempool_id[MEM_CDRAM] = sceKernelAllocMemBlock("cdram_mempool", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, mempool_size[MEM_CDRAM], NULL);

	for (int i = 0; i < MEM_NONE; i++) {
		if (mempool_size[i]) {
			mempool_addr[i] = NULL;
			sceKernelGetMemBlockBase(mempool_id[i], &mempool_addr[i]);

			if (mempool_addr[i]) {
				mempool_mspace[i] = sceClibMspaceCreate(mempool_addr[i], mempool_size[i]);
			}
		}
	}

	mempool_init = 1;
}

void term_memory()
{
	if (!mempool_init)
		return;
	
	for (int i = 0; i < MEM_NONE; i++) {
		sceClibMspaceDestroy(mempool_mspace[i]);
		mempool_mspace[i] = NULL;
		sceKernelFreeMemBlock(mempool_id[i]);
		mempool_addr[i] = NULL;
		mempool_id[i] = 0;
		mempool_size[i] = 0;
	}

	mempool_init = 0;
}

MemType get_mem_type_by_addr(void *addr)
{
	if (addr >= mempool_addr[MEM_PHYCONT] && (addr < mempool_addr[MEM_PHYCONT] + mempool_size[MEM_PHYCONT]))
		return MEM_PHYCONT;
	else if (addr >= mempool_addr[MEM_CDLG] && (addr < mempool_addr[MEM_CDLG] + mempool_size[MEM_CDLG]))
		return MEM_CDLG;
	else if (addr >= mempool_addr[MEM_CDRAM] && (addr < mempool_addr[MEM_CDRAM] + mempool_size[MEM_CDRAM]))
		return MEM_CDRAM;

	return MEM_NONE;
}

void free_hook(void *ptr)
{
	MemType mem_type = get_mem_type_by_addr(ptr);
	if (mem_type == MEM_NONE) {
		__real_free(ptr);
	} else {
		sceClibMspaceFree(mempool_mspace[mem_type], ptr);
	}
}

void *malloc_hook(size_t size)
{
	void *mem = __real_malloc(size);

	if (mem) {
		return mem;
	}

	for (int i = 0; i < MEM_NONE; i++) {
		if (mempool_mspace[i]) {
			mem = sceClibMspaceMalloc(mempool_mspace[i], size);
			if (mem) {
				return mem;
			}
		}
	}

	return NULL;
}

void *calloc_hook(size_t num, size_t size)
{
	void *mem = __real_calloc(num, size);

	if (mem) {
		return mem;
	}

	for (int i = 0; i < MEM_NONE; i++) {
		if (mempool_mspace[i]) {
			mem = sceClibMspaceCalloc(mempool_mspace[i], num, size);
			if (mem) {
				return mem;
			}
		}
	}

	return NULL;
}

void *memalign_hook(size_t alignment, size_t size)
{
	void *mem = __real_memalign(alignment, size);

	if (mem) {
		return mem;
	}

	for (int i = 0; i < MEM_NONE; i++) {
		if (mempool_mspace[i]) {
			mem = sceClibMspaceMemalign(mempool_mspace[i], alignment, size);
			if (mem) {
				return mem;
			}
		}
	}

	return NULL;
}

void *realloc_hook(void *ptr, size_t size)
{
	MemType mem_type = get_mem_type_by_addr(ptr);
	void *mem = NULL;

	if (mem_type == MEM_NONE) {
		mem = __real_realloc(ptr, size);
	} else {
		mem = sceClibMspaceRealloc(mempool_mspace[mem_type], ptr, size);
	}

	if (!mem) {
		mem = malloc_hook(size);

		if (mem) {
			size_t copy_size = 0;

			if (mem_type == MEM_NONE) {
				copy_size = malloc_usable_size(ptr);
			} else {
				SceKernelMemBlockInfo info;
				info.size = sizeof(SceKernelMemBlockInfo);
				sceKernelGetMemBlockInfoByAddr(ptr, &info);
				copy_size = info.mappedSize;
			}

			if (size < copy_size) {
				copy_size = size;
			}

			memcpy(mem, ptr, copy_size);
			free_hook(ptr);
		}
	}

	return mem;
}
