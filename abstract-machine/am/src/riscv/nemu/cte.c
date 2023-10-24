#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      default: ev.event = EVENT_ERROR; break;
    }
    //user_handler是cte_init中注册的回调函数
    c = user_handler(ev, c);
    assert(c != NULL);
  }
  printf("%s",&c);
  return c;
}
//当 extern 用于函数声明时，它表示该函数的定义在当前编译单元之外，将在其他地方提供。这允许在当前编译单元中使用该函数，而不需要提供函数的完整定义。
//声明一个名为 __am_asm_trap 的函数。这个函数是一个汇编语言编写的异常处理入口点，它在上面的 cte_init 函数中被使用。
//__am_asm_trap 函数的地址被用作异常处理入口地址
extern void __am_asm_trap(void);

//Context*(*handler)(Event, Context*这个声明描述了一个函数指针，该指针指向一个函数，该函数接受两参数并返回指向 Context 结构的指针。
bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry        内联汇编,异常处理的入口地址设置为__am_asm_trap。
  //%0 是内联汇编中的操作数占位符，它表示内联汇编指令中的第一个操作数。在这里的内联汇编指令中，%0 用来引用第一个输入操作数，即 "r"(__am_asm_trap) 中的 __am_asm_trap。
  //"r" 约束表示将一个寄存器作为输入操作数
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler 行将用户提供的事件处理程序函数指针handler赋值给全局变量user_handler。这个事件处理程序将在事件发生时被调用，用于处理事件和返回上下文。
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

//li a7, -1: 这部分指令将立即数 -1 加载到寄存器 a7 中。在 RISC-V 架构中，a7 寄存器通常用于传递系统调用号（syscall number）。
//ecall: 这部分指令触发一个自陷操作。在 RISC-V 中，ecall 用于进入特权模式（例如，从用户态进入内核态），并执行相应的系统调用。
void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
