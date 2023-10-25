#include <common.h>

static Context* do_event(Event e, Context* c) {
  printf("%x\n",c->mepc);
  printf("%x\n",c->mepc);
  //  switch (e.event) {
  //    default: panic("Unhandled event ID = %d", e.event);
  //  }
  //返回输入的上下文 c，表示处理事件后的上下文状态。
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
