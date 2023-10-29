#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>
#include "syscall.h"

// helper macros
#define _concat(x, y) x ## y //用来将 x 和 y 连接在一起,动态地生成宏名称
#define concat(x, y) _concat(x, y)
#define _args(n, list) concat(_arg, n) list //n 是参数的数量，list 是参数列表。这个宏会根据参数数量调用不同的 _argN 宏
#define _arg0(a0, ...) a0
#define _arg1(a0, a1, ...) a1
#define _arg2(a0, a1, a2, ...) a2
#define _arg3(a0, a1, a2, a3, ...) a3
#define _arg4(a0, a1, a2, a3, a4, ...) a4
#define _arg5(a0, a1, a2, a3, a4, a5, ...) a5

// extract an argument from the macro array
/*SYSCALL: 这个宏用于提取参数数组 ARGS_ARRAY 中的第 0 个位置的参数，通常用于表示系统调用的类型或标识。

GPR1, GPR2, GPR3, GPR4: 这些宏用于提取参数数组 ARGS_ARRAY 中的第 1、2、3、4 个位置的参数，通常用于表示寄存器值或其他操作数。

GPRx: 这个宏用于提取参数数组 ARGS_ARRAY 中的第 5 个位置的参数，通常用于表示返回值。*/
#define SYSCALL  _args(0, ARGS_ARRAY)
#define GPR1 _args(1, ARGS_ARRAY)
#define GPR2 _args(2, ARGS_ARRAY)
#define GPR3 _args(3, ARGS_ARRAY)
#define GPR4 _args(4, ARGS_ARRAY)
#define GPRx _args(5, ARGS_ARRAY)

// ISA-depedent definitions
#if defined(__ISA_X86__)
# define ARGS_ARRAY ("int $0x80", "eax", "ebx", "ecx", "edx", "eax")
#elif defined(__ISA_MIPS32__)
# define ARGS_ARRAY ("syscall", "v0", "a0", "a1", "a2", "v0")
/*"ecall"：表示系统调用类型，通常用来标识要执行的系统调用操作。
"a7"：通常用于 RISC-V 中表示系统调用号（syscall number），它是一个寄存器，存储着要执行的系统调用的编号。
"a0"、"a1"、"a2"：这些参数表示要传递给系统调用的参数值。在 RISC-V 架构中，这些参数通常存储在 a0、a1、a2 等寄存器中。
"a0"：这是最后一个参数，通常用于表示系统调用的返回值，将存储系统调用的结果。*/
#elif defined(__ISA_RISCV32__) || defined(__ISA_RISCV64__) //riscv32架构的
# define ARGS_ARRAY ("ecall", "a7", "a0", "a1", "a2", "a0")
#elif defined(__ISA_AM_NATIVE__)
# define ARGS_ARRAY ("call *0x100000", "rdi", "rsi", "rdx", "rcx", "rax")
#elif defined(__ISA_X86_64__)
# define ARGS_ARRAY ("int $0x80", "rdi", "rsi", "rdx", "rcx", "rax")
#elif defined(__ISA_LOONGARCH32R__)
# define ARGS_ARRAY ("syscall 0", "a7", "a0", "a1", "a2", "a0")
#else
#error _syscall_ is not implemented
#endif
/*  dummy/main
#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main() {
  return _syscall_(SYS_yield, 0, 0, 0);
}
*/
intptr_t _syscall_(intptr_t type, intptr_t a0, intptr_t a1, intptr_t a2) {
  //使用 register 关键字将 _gpr1、_gpr2、_gpr3、_gpr4 和 ret 分别分配到寄存器中。这些寄存器用于传递参数和接收系统调用的返回值。
  //_gpr1 变量将与 GPR1 寄存器相关联，这个寄存器将用于存储 _gpr1 变量的值
  //GPR的值("syscall 0", "a7", "a0", "a1", "a2", "a0")
  register intptr_t _gpr1 asm (GPR1) = type;//a7
  register intptr_t _gpr2 asm (GPR2) = a0;
  register intptr_t _gpr3 asm (GPR3) = a1;
  register intptr_t _gpr4 asm (GPR4) = a2;
  register intptr_t ret asm (GPRx);//a0
  asm volatile (SYSCALL : "=r" (ret) : "r"(_gpr1), "r"(_gpr2), "r"(_gpr3), "r"(_gpr4));
  
  return ret;
}

void _exit(int status) {
  _syscall_(SYS_exit, status, 0, 0);
  while (1);
}

int _open(const char *path, int flags, mode_t mode) {
  _exit(SYS_open);
  return 0;
}

int _write(int fd, void *buf, size_t count) {
 
  // assert(fd == 1 || fd == 2);

  // assert(count > 0);
  
  // for (int i = 0; i < count; i++) {
  //   putch(*((char*)buf + i));
  // }

  // return count;
}

void *_sbrk(intptr_t increment) {
  return (void *)-1;
}

int _read(int fd, void *buf, size_t count) {
  _exit(SYS_read);
  return 0;
}

int _close(int fd) {
  _exit(SYS_close);
  return 0;
}

off_t _lseek(int fd, off_t offset, int whence) {
  _exit(SYS_lseek);
  return 0;
}

int _gettimeofday(struct timeval *tv, struct timezone *tz) {
  _exit(SYS_gettimeofday);
  return 0;
}

int _execve(const char *fname, char * const argv[], char *const envp[]) {
  _exit(SYS_execve);
  return 0;
}

// Syscalls below are not used in Nanos-lite.
// But to pass linking, they are defined as dummy functions.

int _fstat(int fd, struct stat *buf) {
  return -1;
}

int _stat(const char *fname, struct stat *buf) {
  assert(0);
  return -1;
}

int _kill(int pid, int sig) {
  _exit(-SYS_kill);
  return -1;
}

pid_t _getpid() {
  _exit(-SYS_getpid);
  return 1;
}

pid_t _fork() {
  assert(0);
  return -1;
}

pid_t vfork() {
  assert(0);
  return -1;
}

int _link(const char *d, const char *n) {
  assert(0);
  return -1;
}

int _unlink(const char *n) {
  assert(0);
  return -1;
}

pid_t _wait(int *status) {
  assert(0);
  return -1;
}

clock_t _times(void *buf) {
  assert(0);
  return 0;
}

int pipe(int pipefd[2]) {
  assert(0);
  return -1;
}

int dup(int oldfd) {
  assert(0);
  return -1;
}

int dup2(int oldfd, int newfd) {
  return -1;
}

unsigned int sleep(unsigned int seconds) {
  assert(0);
  return -1;
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
  assert(0);
  return -1;
}

int symlink(const char *target, const char *linkpath) {
  assert(0);
  return -1;
}

int ioctl(int fd, unsigned long request, ...) {
  return -1;
}
