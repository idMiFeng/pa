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

#include <isa.h>
#include <memory/paddr.h>

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
//这些指令将被加载到虚拟计算机的内存中以执行。
static const uint32_t img [] = {
  0x800002b7,  // lui t0,0x80000
  0x0002a023,  // sw  zero,0(t0)
  0x0002a503,  // lw  a0,0(t0)
  0x00100073,  // ebreak (used as nemu_trap)
};

static void restart() {
  /* Set the initial program counter. */
  //cpu.pc = RESET_VECTOR：设置程序计数器（cpu.pc）的值为 RESET_VECTOR。程序计数器存储下一条要执行的指令的地址。
  cpu.pc = RESET_VECTOR;

  /* The zero register is always 0. */
  //cpu.gpr[0] = 0：将通用寄存器组（cpu.gpr）的第一个寄存器（通常是零寄存器）的值设置为零。通用寄存器通常用于存储程序执行过程中的数据。
  cpu.gpr[0] = 0;
}

void init_isa() {
  /* Load built-in image. */
  //把客户程序读入到一个固定的内存位置RESET_VECTOR. RESET_VECTOR的值在nemu/include/memory/paddr.h中定义.s
  //guest_to_host接受虚拟地址返回物理地址，把客户程序加载到真正的物理地址
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  /* Initialize this virtual computer system. */
  //restart()：调用 restart 函数，将虚拟计算机的状态重置，包括将程序计数器设置为起始地址 RESET_VECTOR，并将通用寄存器的第一个寄存器设置为零。
  restart();
}
