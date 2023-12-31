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
#include <common.h>

/****************************************/



#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/paddr.h>//引入paddr_read

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}


static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state=NEMU_QUIT;
  return -1;
  
}

static int cmd_help(char *args);

//单部执行
static int cmd_si(char *args)
{
  int n;
  if (args==NULL){
    n=1;
  }
  else sscanf(args,"%d",&n);
  cpu_exec(n);
  return 0;

}

//打印程序状态
static int cmd_info(char *args){
  if (args==NULL){
    printf("\"r\"-Print register status  or  \"w\"-Print watchpoint information\n");
  }
  else if (strcmp(args, "r") == 0){
    isa_reg_display();
  }
  else if (strcmp(args,"w")==0)
  {
    //打印监视点状态
    info_watchpoint();
  }

  return 0;
}


//扫描内存
static int cmd_x(char *args){
  if (args == NULL) {
        printf("Wrong Command!\n");
        return 0;
    }                                                                           
	int N;
  uint32_t startAddress;
	sscanf(args,"%d%x",&N,&startAddress);
	for (int i = 0;i < N;i ++){
      printf("%x\n", paddr_read(startAddress,4));
      //C语言会自动执行类型提升以匹配表达式的操作数的类型。所以，4 被转换为 uint32_t，
      startAddress += 4;
  
  }
   return 0;
}

//表达式求值
static int cmd_p(char *args){
  bool success=true;
  int32_t res = expr(args, &success);
  if (!success) 
  {
    printf("invalid expression\n");
  } else 
  {
    printf("%d\n", res);
  }
  return 0; 
}

//设置监视点
static int cmd_w(char *args){
 if (!args)
  {
    printf("Usage: w EXPR\n");
    return 0;
  }
  bool success;
  int32_t res = expr(args, &success);
  if (!success) 
  {
    printf("invalid expression\n");
  } 
  else 
  {
    wp_set(args, res);
  }
  return 0;
}

//删除序列号为N的监视点
static int cmd_d(char *args){
  char *arg = strtok(NULL, "");
  if (!arg) {
    printf("Usage: d N\n");
    return 0;
  }
  int no = strtol(arg, NULL, 10);
  wp_remove(no);
  return 0;
}


//可用的命令
static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  {"si","single step",cmd_si},
  {"info","Print program status",cmd_info},
  { "x", "Usage: x N EXPR. Scan the memory from EXPR by N bytes", cmd_x },
  {"p","Expression evaluation",cmd_p},
  { "w", "Usage: w EXPR.Set a watchpoint", cmd_w },
  { "d", "Usage: d N. Delete watchpoint of wp.NO=N", cmd_d },


  /* TODO: Add more commands */

};



#define NR_CMD ARRLEN(cmd_table)


//帮助信息
static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}


//主循环在这里定义//////////////////////////////////////////////////////////////////////////////
void sdb_mainloop() {
  //检查是否处于批处理模式
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }
  //rl_gets()函数获取用户在命令行中输入的文本。这个文本通常包括一个命令和可能的参数。
//用户输入的命令以字符串形式存储在str中，然后使用strtok()函数从字符串中提取第一个标记作为命令（cmd）。
  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    //如果args的起始位置超过了输入文本的末尾位置（args >= str_end），则将参数设为NULL。这可能发生在用户只输入了命令而没有参数的情况下。
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      //如果找到匹配的命令，它会调用相应的处理函数，并将参数传递给它。如果处理函数返回小于0的值，表示需要退出，此时循环结束，程序退出。
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();

  //sdb_set_batch_mode();
}
