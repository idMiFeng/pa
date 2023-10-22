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


#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <common.h>
/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */

///////////////////iringbuf
#define IRING_BUF_SIZE 16       		// 环形缓冲区大小
#define IRING_BUF_PC_START_INDEX 4    	// 存放指令信息的开始位置
static char iringbuf[IRING_BUF_SIZE][128 + IRING_BUF_PC_START_INDEX];   // 环形缓冲区 
static size_t iringbuf_index = 0;    	// 当前指令占据的位置
static void print_iringbuf() {
  char prefix[IRING_BUF_PC_START_INDEX] = "-->";
  prefix[IRING_BUF_PC_START_INDEX-1]='\0';
  for (int i = 0; i < IRING_BUF_SIZE; ++i) {
    if (iringbuf[i][IRING_BUF_PC_START_INDEX] == '\0') {
      break;
    }
    if ((i +1) % IRING_BUF_SIZE == iringbuf_index) {
      strncpy(iringbuf[i], prefix, strlen(prefix)); 
    }
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", iringbuf[i]); }
#endif
    printf("%s\n", iringbuf[i]);
  }
  return;
}

///////////////////




#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));

#ifdef CONFIG_WATCHPOINT
  wp_difftest();
  return 0;
#endif
}


/*在execute函数（在cpu_exec中调用）中调用
主要作用是执行一次指令，并更新PC的值。它接收一个Decode结构体指针s和一个指令的PC值作为参数，
将PC值保存到s->pc和s->snpc中，然后调用isa_exec_once()执行指令，最后将执行后的动态下一条指令的PC值赋给cpu.pc。
整个过程涵盖了指令周期的各个阶段，实现了指令的执行和PC的更新。*/
static void exec_once(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;

  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
  for (i = ilen - 1; i >= 0; i --) {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
   strcpy(iringbuf[iringbuf_index] + IRING_BUF_PC_START_INDEX, s->logbuf);
  iringbuf_index = (iringbuf_index + 1) % IRING_BUF_SIZE;
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
#endif
}



//在cpu_exec中调用
static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}



static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}



void assert_fail_msg() {
  //取指令，物理访问约界时调用
  //display_inst();
  
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
/*这段代码是cpu_exec函数的实现，下面是对其功能的详细说明：
首先，根据传入的参数n和预定义的MAX_INST_TO_PRINT比较，确定是否打印每条指令的执行信息。
接着，根据当前的nemu_state.state状态进行判断：
如果状态为NEMU_END或NEMU_ABORT，输出程序执行已结束的提示信息，并返回函数。
否则，将nemu_state.state设置为NEMU_RUNNING表示程序正在运行。
获取当前时间作为计时器的起始时间。
调用execute(n)函数，执行指定数量的指令。
执行完指定数量的指令后，获取当前时间作为计时器的结束时间，并计算指令执行的时间。
根据nemu_state.state的值进行判断：
如果状态为NEMU_RUNNING，将nemu_state.state设置为NEMU_STOP，表示程序执行已暂停。
如果状态为NEMU_END或NEMU_ABORT，根据具体的状态输出相应的日志信息，提示程序是否执行成功。
如果状态为NEMU_QUIT，执行统计操作。
总体来说，该函数的功能是模拟CPU的工作。它根据给定的指令数量执行相应数量的指令，并根据当前的状态进行相应的处理，包括输出提示信息、设置状态、记录执行时间以及执行统计操作。*/
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }
  ///////初始化环形缓冲区
  iringbuf_index = 0;
for (int i = 0; i < IRING_BUF_SIZE; ++i) {
   memset(iringbuf[i], ' ', IRING_BUF_PC_START_INDEX);  // 前几个位置初始化为空格
   iringbuf[i][IRING_BUF_PC_START_INDEX] = '\0';
}
////////

  uint64_t timer_start = get_time();
  
  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
     print_iringbuf();	
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // statistic()这个函数用于记录关于模拟器性能和运行状态的信息，例如主机运行时间、总指令数和模拟频率。这些信息对于性能分析和调试非常有用。
    case NEMU_QUIT: statistic();
  }
}
