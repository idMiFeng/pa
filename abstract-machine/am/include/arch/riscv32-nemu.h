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

/*const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};*/
//gpr1: a7, gpr2: a0, gpr3: a1, gpr4: a2, gprx: a0
#define GPR1 gpr[17] // a7
#define GPR2 gpr[10]
#define GPR3 gpr[11]
#define GPR4 gpr[12]
#define GPRx gpr[10]

#endif
