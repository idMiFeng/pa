#include <common.h>

static Context* do_event(Event e, Context* c) {
   switch (e.event) {
    case 1:printf("event ID=%d\n",e.event);break;
    default: panic("Unhandled event ID = %d", e.event);break;
   }
  //返回输入的上下文 c，表示处理事件后的上下文状态。
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
