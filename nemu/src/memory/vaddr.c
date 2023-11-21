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


word_t vaddr_ifetch_or_read(vaddr_t addr, int len, int type) {
  int flag = isa_mmu_check(addr, len, type);
  paddr_t paddr = addr;
  switch (flag) {
    case MMU_DIRECT:
        // do nothing
        break;
    case MMU_TRANSLATE:
        paddr = isa_mmu_translate(addr, len, type); // assert in isa_mmu_translate
        break;
    case MMU_FAIL:
        assert(0);
  }
  return paddr_read(paddr, len);
}

word_t vaddr_ifetch(vaddr_t addr, int len) {
  return vaddr_ifetch_or_read(addr, len, MEM_TYPE_IFETCH);
}

word_t vaddr_read(vaddr_t addr, int len) {
  return vaddr_ifetch_or_read(addr, len, MEM_TYPE_READ);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  int flag = isa_mmu_check(addr, len, MEM_TYPE_WRITE);
  paddr_t paddr = addr;
  switch (flag) {
    case MMU_DIRECT:
        // do nothing
        break;
    case MMU_TRANSLATE:
        paddr = isa_mmu_translate(addr, len, MEM_TYPE_WRITE); // assert in isa_mmu_translate
        break;
    case MMU_FAIL:
        assert(0);
  }
  paddr_write(paddr, len, data);
}