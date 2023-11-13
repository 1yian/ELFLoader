#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#define STACK_SIZE 1024 * 1024 * 1 // 1 MB for now
typedef struct ElfInfo {
		uint64_t fd;
		uint64_t base_addr;
		Elf64_Ehdr elf_hdr;

		Elf64_Phdr* program_headers;
		uint64_t argc;
		const char** argv;
		const char** envp;
} ElfInfo;

#ifndef MIN
 #define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
#endif

#define PAGE_SIZE sysconf(_SC_PAGESIZE)

#define MEM_ALIGN(M, U) (void*)((uint64_t)(M) & ~((uint64_t)(U) - 1ull))
#define MEM_OFFSET(M, U) ((uint64_t)(M) & ((uint64_t)(U) - 1ull))
#define MEM_CEIL(M, U) ( MEM_OFFSET(M, U) ? (uint64_t)MEM_ALIGN(M, U) + U : M )

void* map_segment(const int fd, const Elf64_Ehdr* const elf_hdr, const Elf64_Phdr* const it);
int make_prot(const int p_flags);
const Elf64_Phdr* find_program_header(const Elf64_Phdr* const table, const size_t len, int item);
void * make_stack(ElfInfo * info);
void read_elf_header(ElfInfo *info);
void read_program_header(ElfInfo *info);
void jump_to_entry(void * entry_addr, void * stack_ptr);

