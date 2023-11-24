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
#define MPIE_OFFSET 7
#define MIE_OFFSET 3
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  // 保存当前的 MIE 到 MPIE
  cpu.mstatus = (cpu.mstatus & ~(1 << MPIE_OFFSET)) | ((cpu.mstatus >> MIE_OFFSET) & 0x1) << MPIE_OFFSET ;
    
    // 将 MIE 置为 0
  cpu.mstatus &= ~(1 << MIE_OFFSET);
  
  epc+=4;
  
  cpu.csr.mcause = NO;
  cpu.csr.mepc = epc;
   
  return cpu.csr.mtvec;
}



void restore_interrupt() { 
  // 还原之前保存的 MPIE 到 MIE
  cpu.mstatus = (cpu.mstatus & ~(1 << MIE_OFFSET)) | ((cpu.mstatus >> MPIE_OFFSET) & 0x1)<< MIE_OFFSET;
  // 将 MPIE 位置为 1
  cpu.mstatus |= (1 << MPIE_OFFSET);

}



word_t isa_query_intr() {
    if ((cpu.mstatus & (1 << MIE_OFFSET))&& cpu.INTR) { 
        cpu.INTR = false;
        return IRQ_TIMER;
    }
    return INTR_EMPTY; 
}
