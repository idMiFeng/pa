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

#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include <isa.h>

/*这段代码定义了一个名为Decode的结构体。它包含以下成员变量：

pc：表示当前指令的程序计数器（Program Counter）的值，即指令的地址。

snpc：表示静态下一条指令的程序计数器的值，即下一条指令的地址。在执行一条指令之前，将当前指令的地址赋给snpc，作为静态下一条指令的地址。

dnpc：表示动态下一条指令的程序计数器的值，即执行完当前指令后，下一条要执行的指令的地址。这个值会在执行指令的过程中根据指令的具体操作进行计算，并在执行完指令后更新。

isa：类型为ISADecodeInfo的结构体，用于存储与指令集相关的信息。具体的定义可以在nemu/src/isa/$ISA/include/isa-def.h中找到。

logbuf：大小为128的字符数组，用于记录跟踪信息。它是一个条件编译的宏，当定义了CONFIG_ITRACE时才会包含在结构体中。

这个Decode结构体在NEMU中用于存储指令的解码信息以及相关的PC值。它是模拟器中指令执行过程中的重要数据结构之一。*/
typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc; // static next pc
  vaddr_t dnpc; // dynamic next pc
  ISADecodeInfo isa;
  IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode;

// --- pattern matching mechanism ---
/*__attribute__((always_inline))是GCC编译器的函数属性之一，用于指示编译器始终将函数进行内联展开。
内联展开是一种编译器优化技术，它将函数调用处的函数体直接嵌入到调用的位置，而不是通过跳转到函数体执行。这样可以减少函数调用的开销，包括栈帧的创建和销毁、参数传递和返回值处理等。*/
__attribute__((always_inline))
static inline void pattern_decode(const char *str, int len,
    uint64_t *key, uint64_t *mask, uint64_t *shift) {
  uint64_t __key = 0, __mask = 0, __shift = 0;
#define macro(i) \
  if ((i) >= len) goto finish; \
  else { \
    char c = str[i]; \
    if (c != ' ') { \
      Assert(c == '0' || c == '1' || c == '?', \
          "invalid character '%c' in pattern string", c); \
      __key  = (__key  << 1) | (c == '1' ? 1 : 0); \
      __mask = (__mask << 1) | (c == '?' ? 0 : 1); \
      __shift = (c == '?' ? __shift + 1 : 0); \
    } \
  }

#define macro2(i)  macro(i);   macro((i) + 1)
#define macro4(i)  macro2(i);  macro2((i) + 2)
#define macro8(i)  macro4(i);  macro4((i) + 4)
#define macro16(i) macro8(i);  macro8((i) + 8)
#define macro32(i) macro16(i); macro16((i) + 16)
#define macro64(i) macro32(i); macro32((i) + 32)
  macro64(0);
  panic("pattern too long");
#undef macro
finish:
  *key = __key >> __shift;
  *mask = __mask >> __shift;
  *shift = __shift;
}

__attribute__((always_inline))
static inline void pattern_decode_hex(const char *str, int len,
    uint64_t *key, uint64_t *mask, uint64_t *shift) {
  uint64_t __key = 0, __mask = 0, __shift = 0;
#define macro(i) \
  if ((i) >= len) goto finish; \
  else { \
    char c = str[i]; \
    if (c != ' ') { \
      Assert((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || c == '?', \
          "invalid character '%c' in pattern string", c); \
      __key  = (__key  << 4) | (c == '?' ? 0 : (c >= '0' && c <= '9') ? c - '0' : c - 'a' + 10); \
      __mask = (__mask << 4) | (c == '?' ? 0 : 0xf); \
      __shift = (c == '?' ? __shift + 4 : 0); \
    } \
  }

  macro16(0);
  panic("pattern too long");
#undef macro
finish:
  *key = __key >> __shift;
  *mask = __mask >> __shift;
  *shift = __shift;
}


// --- pattern matching wrappers for decode ---
#define INSTPAT(pattern, ...) do { \
  uint64_t key, mask, shift; \
  pattern_decode(pattern, STRLEN(pattern), &key, &mask, &shift); \
  if ((((uint64_t)INSTPAT_INST(s) >> shift) & mask) == key) { \
    INSTPAT_MATCH(s, ##__VA_ARGS__); \
    goto *(__instpat_end); \
  } \
} while (0)


/*该宏定义了一个起始位置的标签，并声明了一个指向该标签的指针__instpat_end。其中，name是传入的参数，用于给标签命名。

&&是GCC编译器提供的标签地址扩展功能，它可以获取标签的地址。

concat(__instpat_end_, name)是一个宏展开操作，将__instpat_end_和name拼接在一起，生成新的标签名称。

整个宏定义的目的是创建一个指向起始标签的指针，并且使用一个特定的命名规则来确保每个模式匹配起始位置的唯一性。

这个起始位置的标签和指针将在后续的模式匹配过程中使用，用于跳转到下一个模式匹配的结束位置。*/
#define INSTPAT_START(name) { const void ** __instpat_end = &&concat(__instpat_end_, name);


#define INSTPAT_END(name)   concat(__instpat_end_, name): ; }

#endif
