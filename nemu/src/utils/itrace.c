
// #include <common.h>


// #define MAX_IRINGBUF 16



// //ItraceNOde记录单条指令pc和内容
// typedef struct{
//     word_t pc;
//     uint32_t inst;
// }ItraceNode;

// //环形缓冲区
// ItraceNode iringbuf[MAX_IRINGBUF];

// int p_cur=0;
// bool full=false;

// //存指令
// void trace_inst(word_t pc,uint32_t inst){
//     iringbuf[p_cur].pc=pc;
//     iringbuf[p_cur].inst=inst;
//     p_cur=(p_cur+1)%MAX_IRINGBUF;
//     full=full || p_cur==0;
// }

// //取指令
// void display_inst(){
//     if(!full && !p_cur){
//         return;
//     }
//      int start = 0;
//     int end = full ? MAX_IRINGBUF : p_cur;

//     // 遍历环形缓冲区，从 start 到 end，打印存储的指令
//     for (int i = start; i < end; i++) {
//         word_t pc = iringbuf[i].pc;
//         uint32_t inst = iringbuf[i].inst;

//         // 在这里你可以按照自己的需求输出指令的信息
//         printf("PC: 0x%08x, Instruction: 0x%08x\n", pc, inst);
//     }
   
// }

// //mtracei设施，在paddr_read()调用
// void display_pread(paddr_t addr,int len){
//     printf("pread at" FMT_PADDR "len=%d\n",addr,len);

// }

// //mtrace设施
// void display_pwrite(paddr_t addr,int len,word_t data){
//     printf("pwrite at" FMT_PADDR "len=%d,data=" FMT_WORD "\n",addr,len,data);
// }



// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <elf.h>

// typedef struct {
//     char name[32];     // func name, 32 should be enough
//     Elf64_Addr addr;   // Address of the symbol
//     unsigned char info;
//     Elf64_Xword size;  // Size of the symbol
// } SymEntry;

// SymEntry *symbol_tbl = NULL; // dynamic allocated
// size_t symbol_count = 0;

// // 解析ELF文件并生成符号表
// void parse_elf(const char *elf_file) {
//     FILE *file = fopen(elf_file, "rb");
//     if (!file) {
//         perror("Failed to open ELF file");
//         return;
//     }

//     Elf64_Ehdr ehdr;
//     fread(&ehdr, 1, sizeof(Elf64_Ehdr), file);

//     // 检查ELF文件头部，确保它是一个有效的ELF文件
//     if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0) {
//         fprintf(stderr, "Not a valid ELF file\n");
//         fclose(file);
//         return;
//     }

//     Elf64_Shdr shdr;
//     fseek(file, ehdr.e_shoff, SEEK_SET);

//     // 查找符号表的节头部
//     for (int i = 0; i < ehdr.e_shnum; i++) {
//         fread(&shdr, 1, sizeof(Elf64_Shdr), file);
//         if (shdr.sh_type == SHT_SYMTAB) {
//             // 找到符号表
//             Elf64_Sym sym;
//             fseek(file, shdr.sh_offset, SEEK_SET);
//             size_t sym_count = shdr.sh_size / sizeof(Elf64_Sym);

//             // 分配内存以保存符号表
//             symbol_tbl = (SymEntry *)malloc(sym_count * sizeof(SymEntry));

//             // 读取符号表中的每个符号
//             for (size_t j = 0; j < sym_count; j++) {
//                 fread(&sym, 1, sizeof(Elf64_Sym), file);
//                 // 将符号信息添加到符号表
//                 strncpy(symbol_tbl[symbol_count].name, (const char *)(file + sym.st_name), 32);
//                 symbol_tbl[symbol_count].addr = sym.st_value;
//                 symbol_tbl[symbol_count].info = sym.st_info;
//                 symbol_tbl[symbol_count].size = sym.st_size;
//                 symbol_count++;
//             }

//             break; // 找到符号表后退出循环
//         }
//     }

//     fclose(file);
// }


// static int call_depth = 0;

// void trace_func_call(paddr_t target) {
//     if (symbol_tbl == NULL) return;

//     ++call_depth;

//     if (call_depth <= 2) return; // 忽略 _trm_init 和 main

//     int i = find_symbol_func(target, true);
//     ftrace_write(FMT_PADDR ": %*scall [%s@" FMT_PADDR "]\n",
//         target,
//         (call_depth - 3) * 2, "",
//         (i >= 0) ? symbol_tbl[i].name : "???",
//         target
//     );
// }

// void trace_func_ret(paddr_t pc) {
//     if (symbol_tbl == NULL) return;

//     if (call_depth <= 2) return; // 忽略 _trm_init 和 main

//     int i = find_symbol_func(pc, false);
//     ftrace_write(FMT_PADDR ": %*sret [%s]\n",
//         pc,
//         (call_depth - 3) * 2, "",
//         (i >= 0) ? symbol_tbl[i].name : "???"
//     );

//     --call_depth;
// }

// // 根据函数地址查找符号表，返回相应的函数名索引
// int find_symbol_func(paddr_t addr, bool is_call) {
//     if (symbol_tbl == NULL) return -1;

//     for (int i = 0; i < symbol_count; ++i) {
//         paddr_t func_addr = symbol_tbl[i].addr;

//         if (is_call) {
//             // 对于函数调用，可以匹配函数的起始地址
//             if (addr == func_addr) {
//                 return i;
//             }
//         } else {
//             // 对于函数返回，可以匹配函数的结束地址
//             paddr_t func_end = func_addr + symbol_tbl[i].size;

//             if (addr == func_end) {
//                 return i;
//             }
//         }
//     }

//     return -1; // 未找到匹配的函数
// }