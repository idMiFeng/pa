
//这段代码的目的是初始化程序的入口点 _start，将 s0 寄存器设置为零，初始化程序的栈指针 sp，然后跳转到初始化函数 _trm_init 来执行 Abstract Machine 的初始化操作

.section entry, "ax"
.globl _start
.type _start, @function

_start:
  mv s0, zero
  la sp, _stack_pointer
  jal _trm_init
