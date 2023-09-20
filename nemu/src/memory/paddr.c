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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>
#include <common.h>
#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};//PG_ALIGNi对齐方式,它告诉编译器将变量或类型的内存地址按照4096字节的边界对齐。这可以提高内存访问的效率
//CONFIG_MSIZE 被定义为 0x8000000,对应的十进制数是 134,217,728。因此，pmem 数组的长度将是 134,217,728 个 uint8_t 类型的元素。
#endif


//paddr_t等价uint64_t
uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }//CONFIG_MBASE起始地址
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }//反过来
/*地址映射是一种将逻辑地址（由CPU生成）映射到物理地址（实际存储器中的地址）的过程对于给定的物理地址 paddr，我们要求的是逻辑地址,该函数通过以下步骤进行映射：
1.计算偏移量：将物理地址 paddr 减去 CONFIG_MBASE。CONFIG_MBASE 是一个基准地址，用于表示内存的起始地址。
2.计算内存位置：将偏移量加上 pmem 的起始地址，即 pmem + paddr - CONFIG_MBASE。这将给出在 pmem 数组中对应于给定物理地址的内存位置。
3.返回内存位置：将计算得到的内存位置作为结果返回给调用者。这样，CPU在访问内存时，实际上是通过访问 pmem 数组中的相应偏移位置来读取或写入数据。*/


//读（被下面的paddr_read函数调用）
static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);//addr为物理地址，通过guest_to_host得到逻辑地址，作为参数传入host_read将地址解引用得到存的值
  return ret;
}

//写
static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}


//用于在地址超出物理内存范围时触发错误
/*panic函数的具体行为取决于编程环境和语言。在许多编程语言和框架中，panic函数会引发一个异常或错误，
并在异常处理机制中进行处理。这可能包括输出错误信息、记录日志、执行清理操作或终止程序的执行。*/
static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}


//根据配置信息初始化物理内存
void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
#ifdef CONFIG_MEM_RANDOM
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
    p[i] = rand();
  }
#endif
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}




/*1.首先，使用in_pmem(addr)函数检查给定的物理地址是否在合法的内存范围内。如果在范围内，说明要读取的地址是在物理内存中，那么调用pmem_read(addr, len)函数来从pmem数组中读取对应地址和长度的内存值，并将其返回。
2.如果给定的物理地址不在合法的内存范围内，那么判断是否启用了CONFIG_DEVICE宏。如果启用了，说明要读取的地址可能是设备的内存映射地址，那么调用mmio_read(addr, len)函数来读取对应设备的内存值，并将其返回。
3.如果既不在物理内存范围内，也没有启用设备内存映射，则调用out_of_bound(addr)函数，该函数会抛出一个错误，表示给定的地址超出了内存范围。
4.最后，如果以上情况都不满足，函数会返回0作为默认值。*/
word_t paddr_read(paddr_t addr, int len) {
  IFDEF(CONFIG_MTRACE,display_pread(addr,len));
  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}



void paddr_write(paddr_t addr, int len, word_t data) {
  IFDEF(CONFIG_MTRACE,display_pwrite(addr,len,data));
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
