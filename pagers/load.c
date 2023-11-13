#include "load.h"

void read_elf_header(ElfInfo *info){
	lseek(info->fd, 0, SEEK_SET);
	read(info->fd, &(info->elf_hdr), sizeof(info->elf_hdr));
	return;
}

void read_program_header(ElfInfo *info){
    size_t total_size = info->elf_hdr.e_phentsize * info->elf_hdr.e_phnum;
    info->program_headers = malloc(total_size);

	lseek(info->fd, info->elf_hdr.e_phoff, SEEK_SET);
	read(info->fd, info->program_headers, total_size);

	for(Elf64_Phdr* it = info->program_headers; it != info->program_headers + info->elf_hdr.e_phnum; ++it) {
		if(it->p_type == PT_LOAD){
			uint64_t mapped_addr = (uint64_t)map_segment(info->fd, &info->elf_hdr, it);
				info->base_addr = MIN(info->base_addr, mapped_addr);
		}
	}
}

void *map_segment(int fd, const Elf64_Ehdr *elf_hdr, const Elf64_Phdr *phdr) {
    // Align the segment address to a page boundary.
    void * aligned_addr = MEM_ALIGN(phdr->p_vaddr, PAGE_SIZE);

    // Calculate the amount of front padding needed to align the segment.
    size_t front_pad = MEM_OFFSET(phdr->p_vaddr, PAGE_SIZE);

    // Calculate the total length of the segment, including front padding.
    size_t len = MEM_CEIL(phdr->p_filesz + front_pad, PAGE_SIZE);

    // Get the protection flags for the segment.
    int prot = make_prot(phdr->p_flags);

    // Determine the mapping flags.
    int flags = elf_hdr->e_type == ET_EXEC ? MAP_PRIVATE | MAP_FIXED : MAP_PRIVATE;

    // Calculate the file offset for the segment.
    const unsigned int file_offset = phdr->p_offset - front_pad;

    // Map the segment into memory.
    void *mapped =
      mmap(aligned_addr, len, prot, flags, fd, file_offset);
    assert(mapped != MAP_FAILED && mapped == aligned_addr);

    // Zero out the front padding.
    memset(mapped, 0, front_pad);

    // If the segment size is greater than the file size, map the remaining
    // bytes as anonymous memory.
    if (phdr->p_memsz > phdr->p_filesz) {
        void *bss = phdr->p_vaddr + phdr->p_filesz;
        size_t sz = MEM_CEIL(bss, PAGE_SIZE) - bss;
        assert(sz >= 0);
        memset(bss, 0, sz);

        bss += sz;
        sz = (phdr->p_memsz - phdr->p_filesz) - sz;
        void *m = mmap(bss, sz, prot, flags | MAP_ANONYMOUS, -1, 0);
        assert(m != MAP_FAILED && m == bss);
        memset(bss, 0, sz);

        len += sz;
    }

  printf("Mapped segment: [%p, %p]", mapped, mapped + len);

  // Return the mapped segment address, minus the front padding.
  return mapped + front_pad;
}

int make_prot(const int p_flags) {
	int prot = 0;

	if (p_flags & PF_R)
		prot |= PROT_READ;
	if (p_flags & PF_W)
		prot |= PROT_WRITE;
	if (p_flags & PF_X)
		prot |= PROT_EXEC;
	return prot;
}

void * allocate_stack(){
    void * stack_ptr = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(stack_ptr != MAP_FAILED);
    return stack_ptr + STACK_SIZE;
}

void * make_stack(ElfInfo * info) {
	void * stack_ptr = allocate_stack();

	char ** auxv = info->envp; while(*auxv++);
	{
	   Elf64_auxv_t * it = auxv;
		size_t auxc = 0;	// Includes AT_NULL element
		for(it = auxv; it->a_type != AT_NULL ; ++it, auxc++) {
			switch(it->a_type) {
				case AT_BASE:
					it->a_un.a_val = info->base_addr;
					break;
				case AT_EXECFN:
					it->a_un.a_val = (uint64_t)info->argv[0];
					break;
				case AT_EXECFD:
					it->a_un.a_val = info->fd;
					break;
				case AT_ENTRY:
					it->a_un.a_val = info->elf_hdr.e_entry;
					break;
				case AT_PHNUM:
					it->a_un.a_val = info->elf_hdr.e_phnum;
					break;
				case AT_PHENT:
					it->a_un.a_val = info->elf_hdr.e_phentsize;
					break;
				case AT_PHDR:
					it->a_un.a_val = info->base_addr + info->elf_hdr.e_phoff;
					break;
			}
		}
		stack_ptr -= (auxc) * sizeof(Elf64_auxv_t);
		memmove(stack_ptr, auxv, (auxc) * sizeof(Elf64_auxv_t));
	}

	const size_t envc = (const char**)auxv - info->envp;	// Includes NULL

	// copy envp to stack
	stack_ptr -= envc * sizeof(char*);
	memmove(stack_ptr, info->envp, envc * sizeof(char*));

	// copy argv to stack
	stack_ptr -= (info->argc + 1) * sizeof(char*);
	memmove(stack_ptr, info->argv, (info->argc + 1) * sizeof(char*));

	// copy argc to stack
	stack_ptr -= sizeof(info->argc);
	memmove(stack_ptr, &info->argc, sizeof(info->argc));


	return stack_ptr;
}

void jump_to_entry(void * entry_addr, void * stack_ptr){
    	asm __volatile__(
			"movq %1, %%rsp\n\t"
			"movq %0, %%rbp\n\t"

			"xor %%rax, %%rax\n\t"
			"xor %%rbx, %%rbx\n\t"
			"xor %%rcx, %%rcx\n\t"
			"xor %%rdx, %%rdx\n\t"
			"xor %%rsi, %%rsi\n\t"
			"xor %%rdi, %%rdi\n\t"
			"xor %%r8, %%r8\n\t"
			"xor %%r9, %%r9\n\t"
			"xor %%r10, %%r10\n\t"
			"xor %%r11, %%r11\n\t"
			"xor %%r12, %%r12\n\t"
			"xor %%r13, %%r13\n\t"
			"xor %%r14, %%r14\n\t"
			"xor %%r15, %%r15\n\t"

			"jmp *%%rbp\n\t"
			:
			: "r" (entry_addr), "r" (stack_ptr)
		);
}
