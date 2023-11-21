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
#include <memory/vaddr.h>

#define VA_OFFSET(addr) (addr & 0x00000FFF)
#define VA_VPN_0(addr)  ((addr >> 12) & 0x000003FF)
#define VA_VPN_1(addr)  ((addr >> 22) & 0x000003FF)

#define PTE_V(item)   (item & 0x1)
#define PTE_R(item)   ((item >> 1) & 0x1)
#define PTE_W(item)   ((item >> 2) & 0x1)
#define PTE_X(item)   ((item >> 3) & 0x1)
#define PTE_PPN(item) ((item >> 12) & 0xFFFFF) 

typedef vaddr_t PTE;
typedef uint32_t rtlreg_t;
paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  // 从 satp 寄存器获取页表基址
  rtlreg_t satp = cpu.csr.satp;
  PTE page_dir_base = satp << 12;
  // 提取虚拟地址中的偏移、一级页号和二级页号
  uint32_t offset = VA_OFFSET(vaddr);
  uint32_t vpn_1 = VA_VPN_1(vaddr);
  uint32_t vpn_0 = VA_VPN_0(vaddr);
  // 计算一级页表项的地址
  PTE page_dir_target = page_dir_base + vpn_1 * 4;
  // 读取一级页表项的内容
  word_t page_dir_target_item = paddr_read(page_dir_target, 4);
  // 检查一级页表项是否有效，否则断言失败
  if (PTE_V(page_dir_target_item) == 0) assert(0);
  // 计算二级页表的基址
  PTE page_table_base = PTE_PPN(page_dir_target_item) << 12;
  // 计算二级页表项的地址
  PTE page_table_target = page_table_base + vpn_0 * 4;
  // 读取二级页表项的内容
  word_t page_table_target_item = paddr_read(page_table_target, 4);
  // 检查二级页表项是否有效，否则断言失败
  if (PTE_V(page_table_target_item) == 0) assert(0);
  // 根据访存类型检查权限位
  switch (type) {
    case MEM_TYPE_IFETCH: if (PTE_X(page_table_target_item) == 0) assert(0); break;
    case MEM_TYPE_READ:   if (PTE_R(page_table_target_item) == 0) assert(0); break;
    case MEM_TYPE_WRITE:  if (PTE_W(page_table_target_item) == 0) assert(0); break;
    default: assert(0); break;
  }
  // 计算物理地址
  paddr_t ppn = PTE_PPN(page_table_target_item) << 12;
  paddr_t paddr = ppn | offset;
  // 检查计算的物理地址是否与虚拟地址相等，否则断言失败
  assert(paddr == vaddr);

  return paddr;
}
