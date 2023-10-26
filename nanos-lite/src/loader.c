#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
      // 获取可执行文件的大小
    size_t file_size = get_ramdisk_size();

    // 分配足够的内存来存储可执行文件
    void *program_memory = malloc(file_size);

    // 从ramdisk中读取可执行文件内容
    ramdisk_read(program_memory, 0, file_size);

    // 获取可执行文件头部信息
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)program_memory;

    // 检查ELF标识符，确保这是一个有效的ELF文件
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        // 非法ELF文件，释放内存并返回
        free(program_memory);
        return;
    }

    // 设置程序的入口点
    uintptr_t entry_point = ehdr->e_entry;
      //返回ELF文件的入口地址，表示加载并准备执行的程序的入口点。
      return entry_point;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}
