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

#include "sdb.h"

// wp_pool 数组可以存储 NR_WP 个监视点结构体对象
#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char *expression;  // 监视点的表达式
  int value;        // 监视点的上一个值

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
//head用于组织使用中的监视点结构, free_用于组织空闲的监视点结构
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

//new_wp()从free_链表中返回一个空闲的监视点结构
WP* new_wp();

//free_wp()将wp归还到free_链表中
void free_wp(WP *wp);

WP* new_wp()
{
  if(free_==NULL)
  {
    printf("free_没有空闲监视点\n");
    assert(0);
  }  
  WP *pos=free_;
  free_++;
  pos->next=head;
  head=pos;
  return pos;
}


void free_wp(WP *wp)
{
  if(wp==head)
  {
    head=head->next;
  }
  else
  {
    WP *pos=head;
    while(pos && pos->next!=wp)
    {
      pos++;
    }
    if (!pos) 
    {
      printf("输入的监视点不在head链表中\n");
      assert(0);
    }
    pos->next=wp->next;  
  }
  wp->next=free_;
  free_=wp;
}


void info_watchpoint()
{
  WP *pos=head;
  if(!pos)
  {
    printf("NO watchpoints");
    return;
  }
   printf("%-8s%-8s\n", "No", "Expression");
  while (pos) {
    printf("%-8d%-8s\n", pos->NO, pos->expression);
    pos = pos->next;
  }
}



void wp_set(char *args, int32_t res)
{
  WP* wp = new_wp();
  strcpy(wp->expression, args);
  wp->value=res;
  printf("Watchpoint %d: %s\n", wp->NO, wp->expression);
}


void wp_remove(int no)
{
  if(no<0 || no>=NR_WP)
  {
    printf("N is not in right\n");
    assert(0);
  }
  WP* wp = &wp_pool[no];
  free_wp(wp);
  printf("Delete watchpoint %d: %s\n", wp->NO, wp->expression);
}


void wp_difftest()
{
   WP* pos = head;
  while (pos) {
    bool _;
    word_t new = expr(pos->expression, &_);
    if (pos->value != new) {
      printf("Watchpoint %d: %s\n"
        "Old value = %d\n"
        "New value = %d\n"
        , pos->NO, pos->expression, pos->value, new);
      pos->value = new;
      nemu_state.state=NEMU_STOP;
    }
    pos = pos->next;
  }
}