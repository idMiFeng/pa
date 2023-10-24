#ifndef ARCH_H__
#define ARCH_H__

struct Context {
  // TODO: fix the order of these members to match trap.S 
  //gpr为通用寄存器，mcause为csr异常号存储寄存器，mstatus为csr状态寄存器，mepc为异常地址寄存器
  //通过观察trap.s中我们发现上下文存储的方式为先存储32个通用寄存器随后存储mcause mstatus mepc
  uintptr_t gpr[32];
  uintptr_t mcause, mstatus, mepc;
  void *pdir;
  uintptr_t np;
};


#define GPR1 gpr[17] // a7
#define GPR2 gpr[0]
#define GPR3 gpr[0]
#define GPR4 gpr[0]
#define GPRx gpr[0]

#endif
