#include "load.h"

#define __USE_GNU

#include <elf.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ucontext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define NUM_PAGES_ON_FAULT 2

static void install_segv_handler();
static void map_page(void * fault_addr);
static void read_program_header_fault(ElfInfo *info);
void map_fault_and_next_page(Elf64_Phdr *phdrs, int phnum, void *faulty_addr);
ElfInfo info;


void user_execve(int argc, char* argv[], char* envp[]) {

	info.fd = open(argv[0], O_RDONLY),
	info.argc = argc,
	info.argv = argv,
	info.envp = envp,
	info.base_addr = UINT64_MAX,
    read_elf_header(&info);
    read_program_header_fault(&info);
	install_segv_handler();
	map_page(info.elf_hdr.e_entry);
	make_stack(&info);
	void * stack_ptr = make_stack(&info);
    jump_to_entry((void *)info.elf_hdr.e_entry, stack_ptr);
	return;
}

static void read_program_header_fault(ElfInfo *info){
    size_t total_size = info->elf_hdr.e_phentsize * info->elf_hdr.e_phnum;
    info->program_headers = malloc(total_size);

	lseek(info->fd, info->elf_hdr.e_phoff, SEEK_SET);
	read(info->fd, info->program_headers, total_size);

	for(Elf64_Phdr* it = info->program_headers; it != info->program_headers + info->elf_hdr.e_phnum; ++it) {
		if(it->p_type == PT_LOAD){
				info->base_addr = MIN(info->base_addr, it->p_vaddr);
		}
	}
}

bool is_valid_address(uint64_t addr) {
    for (Elf64_Phdr* phdr = info.program_headers; phdr < info.program_headers + info.elf_hdr.e_phnum; ++phdr) {
        if (phdr->p_type == PT_LOAD) {
            uint64_t start = phdr->p_vaddr;
            uint64_t end = start + phdr->p_memsz;

            if (addr >= start && addr < end) {
                return true;
            }
        }
    }
    return false;
}


static void segv_handler(int signo, siginfo_t *sinfo, void *_context) {
    if (signo != SIGSEGV || sinfo->si_code != SEGV_MAPERR) {
        goto reinitialize_signal;
    }

    uint64_t faulty_addr = (uint64_t)sinfo->si_addr;
    printf("segfault at %p\n", faulty_addr);

    // Additional check for valid range (adjust according to your application's memory layout)
    if (!is_valid_address(faulty_addr)) {
        goto reinitialize_signal;
    }

    if (faulty_addr && faulty_addr >= PAGE_SIZE) {
        map_fault_and_next_page(info.program_headers, info.elf_hdr.e_phnum, faulty_addr);
        return;
    }

reinitialize_signal:
    // Consider alternative actions or logging here
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, NULL);
}

static void install_segv_handler() {
    struct sigaction sa;

    // Set the handler function for SIGSEGV
    sa.sa_sigaction = segv_handler;

    // Initialize the signal set and add SIGSEGV to it
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGSEGV);

    // Set flags to use the sa_sigaction field, not sa_handler
    sa.sa_flags = SA_SIGINFO;

    // Register the signal handler for SIGSEGV
    sigaction(SIGSEGV, &sa, NULL);
}

// Function to find the next page address in the same segment
static void *find_next_page_address(Elf64_Phdr *phdrs, int phnum, void *faulty_addr) {
    for (int i = 0; i < phnum; i++) {
        Elf64_Phdr *phdr = &phdrs[i];
        uint64_t seg_start = phdr->p_vaddr;
        uint64_t seg_end = seg_start + phdr->p_memsz;

        if ((uint64_t)faulty_addr >= seg_start && (uint64_t)faulty_addr < seg_end) {
            uint64_t next_page_start = ((uint64_t)faulty_addr + PAGE_SIZE) & ~(PAGE_SIZE - 1);
            if (next_page_start < seg_end) {
                return (void *)next_page_start;
            }
            break;
        }
    }
    return NULL;
}

// Function to map the fault address and the next page
void map_fault_and_next_page(Elf64_Phdr *phdrs, int phnum, void *faulty_addr) {
    printf("Mapping %p\n", faulty_addr);
    map_page(faulty_addr); // Map the faulting address
    void * tmp = faulty_addr;

    for (int i = 0; i < NUM_PAGES_ON_FAULT - 1; i++){
        void *next_page_addr = find_next_page_address(phdrs, phnum, tmp);
        if (next_page_addr) {
            printf("Mapping (next) %p\n", next_page_addr);
            map_page(next_page_addr); // Map the next page if it exists
        } else {
            break;
        }
        tmp = next_page_addr;
    }
}

static void map_page(void * faulty_addr) {
	int flags = info.elf_hdr.e_type == ET_EXEC ? MAP_PRIVATE | MAP_FIXED_NOREPLACE : MAP_PRIVATE;

	for (Elf64_Phdr* it = info.program_headers; it != info.program_headers + info.elf_hdr.e_phnum; ++it) {
		const uint64_t seg_begin = it->p_vaddr,
			  seg_end = it->p_vaddr + it->p_memsz,
			  aligned_begin = MEM_ALIGN(faulty_addr, PAGE_SIZE),
			  bss_begin = it->p_vaddr + it->p_filesz,
			  aligned_end = aligned_begin + PAGE_SIZE;

		const int prot = make_prot(it->p_flags);
		size_t file_offset = 0;
		int fd = 0;

		if ( seg_begin <= faulty_addr && faulty_addr < seg_end ) {
			if (it->p_memsz > it->p_filesz && bss_begin <= aligned_begin) {
				// .bss section
				fd = -1;
				file_offset = 0;
				flags |= MAP_ANONYMOUS;
			} else {
				fd = info.fd;
				file_offset = it->p_offset + aligned_begin - it->p_vaddr;
			}
			printf("Mmapping from %p to %p\n", aligned_begin, aligned_begin + PAGE_SIZE);
			void * result = mmap((void*)aligned_begin, PAGE_SIZE, prot, flags, fd, file_offset);
            if (result == MAP_FAILED) {
                printf("mmap failed\n");
            }



			if (bss_begin <= aligned_begin) {
				memset((void*)aligned_begin, 0, PAGE_SIZE);
			} else {

				if (aligned_begin < it->p_vaddr) {
					memset((void*)aligned_begin, 0, it->p_vaddr - aligned_begin);
				}
				if (it->p_filesz < it->p_memsz && bss_begin < aligned_end) {
					memset((void*)bss_begin, 0, aligned_end - bss_begin);
				}
			}

			break;
		}
	}
}