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

#ifndef __CPU_IFETCH_H__

#include <memory/vaddr.h>


/*该函数接受两个参数：一个指向vaddr_t类型的指针pc和一个表示指令长度的整数len。

函数的目的是从指定的地址(*pc)处获取指定长度(len)的指令，并将地址指针pc向前移动。

代码的执行流程如下：

vaddr_ifetch(*pc, len)：调用vaddr_ifetch函数，根据指定的地址和长度从内存中获取指令的值。具体的实现可能在其他代码文件中定义和实现。
(*pc) += len：将地址指针pc向前移动len个字节，以便下次获取指令时可以从正确的地址开始。
返回获取到的指令值。
inst_fetch函数的返回值是从内存中获取的指令值，用于后续的指令解码和执行过程。通过每次移动地址指针pc，可以按顺序从内存中获取连续的指令。*/
static inline uint32_t inst_fetch(vaddr_t *pc, int len) {
  uint32_t inst = vaddr_ifetch(*pc, len);
  (*pc) += len;
  return inst;
}

#endif

