#include <am.h>
#include <nemu.h>

extern char _heap_start;
int main(const char *args);

Area heap = RANGE(&_heap_start, PMEM_END);//表示堆内存的范围。它的起始地址是 _heap_start，结束地址是 PMEM_END。
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;//存储传递给 main 函数的参数。该参数字符串从上面的 MAINARGS 宏中获取。

void putch(char ch) {
  outb(SERIAL_PORT, ch);//向串行端口发送一个字符 ch
}

void halt(int code) {
  nemu_trap(code);//asm volatile("mv a0, %0; ebreak" : :"r"(code))

  // should not reach here
  while (1);
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
