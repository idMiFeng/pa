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
  
  int fd = fs_open(filename, 0, 0);
  if (fd < 0) {
    panic("should not reach here");
  }
  Elf_Ehdr elf;

  assert(fs_read(fd, &elf, sizeof(elf)) == sizeof(elf));
  // 检查魔数
  assert(*(uint32_t *)elf.e_ident == 0x464c457f);
  
  Elf_Phdr phdr;
  for (int i = 0; i < elf.e_phnum; i++) {
    uint32_t base = elf.e_phoff + i * elf.e_phentsize;

    fs_lseek(fd, base, 0);
    assert(fs_read(fd, &phdr, elf.e_phentsize) == elf.e_phentsize);
    
    // 需要装载的段
    if (phdr.p_type == PT_LOAD) {

      char * buf_malloc = (char *)malloc(phdr.p_filesz);

      fs_lseek(fd, phdr.p_offset, 0);
      assert(fs_read(fd, buf_malloc, phdr.p_filesz) == phdr.p_filesz);
      
      memcpy((void*)phdr.p_vaddr, buf_malloc, phdr.p_filesz);
      memset((void*)phdr.p_vaddr + phdr.p_filesz, 0, phdr.p_memsz - phdr.p_filesz);
      
      free(buf_malloc);
    }
  }

  assert(fs_close(fd) == 0);
  
  return elf.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}


void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
    // 定义用户栈的区域
    Area stack;
    stack.start = pcb->stack;
    stack.end = pcb->stack + STACK_SIZE;

    // 调用 loader 函数加载用户程序，获取入口地址
    uintptr_t entry = loader(pcb, filename);

    // 计算 argc、envc 的值
    int argc = 0;
    while (argv[argc] != NULL) argc++;
    int envc = 0;
    while (envp[envc] != NULL) envc++;

    // 分配用户栈空间，用于存储 argv 和 envp 指针
    uintptr_t* user_stack = (uintptr_t*)heap.end;

    // 将 argv 字符串逆序拷贝到用户栈
    for (int i = argc - 1; i >= 0; i--) {
        size_t len = strlen(argv[i]) + 1;  // 包括 null 终止符
        user_stack -= len;
        strncpy((char*)user_stack, argv[i], len);
    }

    // 对齐到 uintptr_t 边界
    user_stack = (uintptr_t*)((uintptr_t)user_stack & ~(sizeof(uintptr_t) - 1));

    // 将 envp 字符串逆序拷贝到用户栈
    for (int i = envc - 1; i >= 0; i--) {
        size_t len = strlen(envp[i]) + 1;  // 包括 null 终止符
        user_stack -= len;
        strncpy((char*)user_stack, envp[i], len);
    }

    // 对齐到 uintptr_t 边界
    user_stack = (uintptr_t*)((uintptr_t)user_stack & ~(sizeof(uintptr_t) - 1));

    // 将 argv 和 envp 指针拷贝到用户栈
    user_stack -= (argc + envc + 4);  // +4 为 NULL 结尾和 argc/envc 的值
    uintptr_t* user_argv = user_stack;

    // 设置 argc 的值
    user_stack[0] = argc;

    // 设置 argv 指针
    for (int i = 0; i < argc; i++) {
        user_stack[i + 1] = (uintptr_t)heap.end - (argc - i - 1) * sizeof(uintptr_t);
    }

    // 设置 argv 的 NULL 终止符
    user_stack[argc + 1] = 0;

    // 设置 envc 的值
    user_stack[argc + 2] = envc;

    // 设置 envp 指针
    for (int i = 0; i < envc; i++) {
        user_stack[argc + 3 + i] = (uintptr_t)heap.end - (argc + 3 + envc - i - 1) * sizeof(uintptr_t);
    }

    // 设置 envp 的 NULL 终止符
    user_stack[argc + 3 + envc] = 0;

    // 调用 ucontext 函数创建用户上下文，传入入口地址和用户栈
    pcb->cp = ucontext(pcb->cp, stack, (void*)entry);

    // 将用户栈的顶部地址赋给 GPRx 寄存器
    pcb->cp->GPRx = (uintptr_t)user_stack;
}

  /*Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *uctx = (Context *)(kstack.end-sizeof(Context));
  uctx->mepc=(uintptr_t) entry;
  return uctx;
}*/

