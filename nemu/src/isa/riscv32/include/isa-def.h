/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __ISA_RISCV32_H__
#define __ISA_RISCV32_H__

#include <common.h>


/*
riscv32提供ecall指令作为自陷指令, 并提供一个mtvec寄存器来存放异常入口地址. 为了保存程序当前的状态, 
riscv32提供了一些特殊的系统寄存器, 叫控制状态寄存器(CSR寄存器). 在PA中, 我们只使用如下3个CSR寄存器:

mepc寄存器 - 存放触发异常的PC
mstatus寄存器 - 存放处理器的状态
mcause寄存器 - 存放触发异常的原因
riscv32触发异常后硬件的响应过程如下:

将当前PC值保存到mepc寄存器
在mcause寄存器中设置异常号
从mtvec寄存器中取出异常入口地址
跳转到异常入口地址*/
typedef struct {
  word_t mcause;
  vaddr_t mepc;
  word_t mstatus;
  word_t mtvec;
  word_t satp;
} riscv32_CSRs;


typedef struct {
  word_t gpr[32];
  vaddr_t pc;
  riscv32_CSRs csr;
  bool INTR;    
} riscv32_CPU_state;

// decode
/*这段代码定义了一个名为riscv32_ISADecodeInfo的结构体。它包含一个成员变量inst，是一个union类型，用于存储指令的值。
inst：是一个union类型，包含一个成员变量val，类型为uint32_t，用于存储指令的32位值。
这个结构体主要用于存储RISC-V 32位指令集相关的解码信息。它在指令解析的过程中被使用，以便能够获取指令的具体操作码和操作数等信息。
*/
typedef struct {
  union {
    uint32_t val;
  } inst;
} riscv32_ISADecodeInfo;

//#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)
#define isa_mmu_check(vaddr, len, type) ((((cpu.csr.satp & 0x80000000) >> 31) == 1) ? MMU_TRANSLATE : MMU_DIRECT)

#endif
