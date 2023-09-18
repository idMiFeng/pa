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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
  TYPE_I, TYPE_U, TYPE_S,
  TYPE_N, TYPE_R,TYPE_J// none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = SEXT(BITS(i, 31, 31), 1) << 19 | ((SEXT(BITS(i, 19, 12), 8) << 56) >> 56) << 11 | ((SEXT(BITS(i, 20, 20), 1) << 63) >> 63) << 10 | (((SEXT(BITS(i, 30, 21), 10) << 54) >> 54) << 22) >> 22; *imm = *imm << 1; } while (0)


/*在decode_exec函数中调用
首先从 s->isa.inst.val 中获取当前指令的值，存储在变量 i 中。
使用 BITS 宏从指令中提取出相应的字段值。例如，BITS(i, 19, 15) 表示从指令的第 19 位到第 15 位提取出一个字段值，存储在变量 rs1 中。
将 BITS(i, 11, 7) 的字段值赋给 *rd，即将目标操作数的寄存器编号存储在 rd 指针指向的位置。
根据指令的类型 type 进行不同的操作数解析：
如果 type 是 TYPE_I，则调用 src1R() 宏将源操作数1的值存储在 *src1 中，调用 immI() 宏将立即数的值存储在 *imm 中。
如果 type 是 TYPE_U，则调用 immU() 宏将立即数的值存储在 *imm 中。
如果 type 是 TYPE_S，则调用 src1R() 宏将源操作数1的值存储在 *src1 中，调用 src2R() 宏将源操作数2的值存储在 *src2 中，调用 immS() 宏将立即数的值存储在 *imm 中。
总体来说，这段代码根据指令的类型解析指令的操作数。根据不同的指令类型，从指令中提取出对应的字段值，并将其存储在相应的变量中，以便后续使用。*/
static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  //
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break; //框架代码定义了src1R()和src2R()两个辅助宏, 用于寄存器的读取结果记录到相应的操作数变量中
    case TYPE_U:                   immU(); break;//immI等辅助宏, 用于从指令中抽取出立即数
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J:                   immJ(); break; 
  }
}

/*在函数isa_exec_once最后调用，函数isa_exec_once的返回值是decode_exec函数的返回值，用于指示指令执行的结果。具体的解码和执行过程在decode_exec函数中实现
在decode_exec()函数中，首先进行一些初始化操作。
然后，使用宏定义的方式定义了一些指令模式匹配规则。
接下来，根据当前指令的二进制编码，与模式字符串进行匹配。
如果匹配成功，根据指令的类型和操作数，执行相应的操作。
如果匹配失败，则继续匹配下一条指令的模式。
最后，将零寄存器的值重置为0，并返回0。*/
static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;//首先定义了一些局部变量 rd、src1、src2 和 imm，用于存储解码过程中的操作数值。
  s->dnpc = s->snpc;//将 s->snpc 的值赋给 s->dnpc，即将下一条指令的地址保存到 dnpc 中。

//INSTPAT_INST(s) 宏定义将 s->isa.inst.val 作为宏展开的结果，用于获取指令的值。
#define INSTPAT_INST(s) ((s)->isa.inst.val)

//INSTPAT_MATCH(s, name, type, ...) 宏定义用于进行指令模式匹配和执行。该宏接受多个参数，
//其中 s 表示指向 Decode 结构体的指针，name 表示指令的名称，type 表示指令的类型，... 表示可变参数，用于指定执行的操作。
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
  }
  //__VA_ARGS__ 是在宏中表示可变参数的特殊标识符。它允许宏接受可变数量的参数，并在宏的展开中将这些参数传递给其他函数或进行其他操作。
  //具体来说，当在宏的定义中使用 __VA_ARGS__ 时，它会在宏展开时替换为传递给宏的实际参数。这样，可以在宏的定义中将参数列表作为可变参数处理。



  INSTPAT_START();//指示指令模式匹配的开始。INSTPAT(模式字符串, 指令名称, 指令类型, 指令执行操作);
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));

  // 添加对 li 指令的支持
  INSTPAT("??????? ????? ????? ??? ????? 00100 11", li     , I, R(rd) = src1+imm);

  // 添加对 mv 指令的支持
  INSTPAT("??????? ????? ????? 000 ????? ????? 11", mv     , R, R(rd) = src2);

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  
  // 添加对 auipc 指令的支持
  INSTPAT("??????? ????? ????? ?????? 00101 11", auipc, U, R(rd) = s->pc + imm);

  // 添加对 addi 指令的支持
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi, I, R(rd) =src1 + imm);

    // 添加对 jal 指令的支持
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal, J,  R(rd) = s->snpc; s->dnpc = s->pc + imm);
  
  //无效指令
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();//指示指令模式匹配的结束

  R(0) = 0; // reset $zero to 0

  return 0;
}

/*理解上面宏展开后的代码

{ const void ** __instpat_end = &&__instpat_end_;
do {
  uint32_t key, mask, shift;
  pattern_decode("??????? ????? ????? ??? ????? 01101 11", 38, &key, &mask, &shift);
  if (((s->isa.inst.val >> shift) & mask) == key) {
    {
      decode_operand(s, &dest, &src1, &src2, &imm, TYPE_U);
      R(dest) = imm;
    }
    goto *(__instpat_end);
  }
} while (0);
// ...
__instpat_end_: ; }
在这段代码中，首先定义了一个局部变量__instpat_end，用于存储标签地址。

接着使用do { ... } while (0)包裹起来，形成一个循环体。

在循环体中，定义了三个局部变量key、mask和shift，用于存储模式解析的结果。

调用pattern_decode函数将模式字符串解析成key、mask和shift。
key：表示指令编码中关键位的值。在模式匹配中，我们希望通过比较指令的编码和模式字符串的编码来确定是否匹配。key记录了模式字符串中关键位的期望值，用于与指令编码进行比较。

mask：表示指令编码中需要匹配的位的掩码。模式匹配时，我们通常只关注指令编码中特定位置的比特位，其他位置的值可以是任意的。mask用于标识哪些位是需要进行匹配的，1表示需要匹配的位，0表示可以任意值的位。

shift：表示指令编码中关键位距离最低位的偏移量。由于指令编码的不同位可能表示不同的意义，我们需要知道关键位相对于最低位的位置。shift的值表示关键位相对于最低位的偏移量，可以用于从指令编码中提取关键位的值。

通过解析模式字符串并提取出key、mask和shift这些信息，我们就可以将模式匹配和译码的过程更加抽象化和灵活化。通过比较指令编码的关键位与key的值，使用掩码mask确定哪些位需要匹配，然后根据shift的值从指令编码中提取出关键位的值，我们可以进行指令的匹配和译码操作。这种模式匹配的方式可以更加清晰地描述指令的编码格式，并且具有较好的可扩展性和灵活性，适用于不同的指令集架构。

接下来，通过位运算判断当前指令的部分字段是否匹配模式字符串的解析结果。

如果匹配成功，则执行指令的操作，并跳转到标签__instpat_end，结束模式匹配。

最后，通过标签__instpat_end_来定义一个空语句。

总结起来，模式匹配机制通过解析模式字符串，将其转换为关键字、掩码和位移量，然后根据位运算进行指令的匹配。当指令匹配成功时，执行相应的操作，并跳转到标签结束模式匹配。这种机制使得指令的译码和执行操作可以通过模式匹配的方式进行灵活的定义和扩展。
*/




/*在exec_once中调用
这段代码是一个函数isa_exec_once的实现，它接受一个指向Decode结构体的指针s作为参数。函数的目的是执行一条指令，其中包括指令的取指和解码过程。

代码的执行流程如下：

inst_fetch(&s->snpc, 4)：调用inst_fetch函数，根据s->snpc（static next pc）获取指令的值。函数的第二个参数4表示要取4字节的指令。
s->isa.inst.val = ...：将获取到的指令值存储到s->isa.inst.val中，即存储到Decode结构体的isa成员的inst联合体中的val成员。
decode_exec(s)：调用decode_exec函数，将指令解码并执行。该函数会根据s中存储的指令信息执行相应的操作，并返回执行的结果。
函数isa_exec_once的返回值是decode_exec函数的返回值，用于指示指令执行的结果。具体的解码和执行过程在decode_exec函数中实现，*/
int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
