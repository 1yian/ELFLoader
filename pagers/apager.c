#include <fcntl.h>    // For open, O_RDONLY
#include <stdint.h>   // For UINT64_MAX
#include "load.h"

void user_execve(int argc, char* argv[], char* envp[]) {
	ElfInfo info;
	info.fd = open(argv[0], O_RDONLY),
	info.argc = argc,
	info.argv = argv,
	info.envp = envp,
	info.base_addr = UINT64_MAX,
    read_elf_header(&info);
    read_program_header(&info);
	void * stack_ptr = make_stack(&info);
    jump_to_entry((void *)info.elf_hdr.e_entry, stack_ptr);
}
