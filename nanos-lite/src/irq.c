#include <common.h>

static Context* do_event(Event e, Context* c) {
  printf("do_event中e.event=%d\n",e.event);
   switch (e.event) {
    case 1:printf("event ID=%d\nc->GPRx=%d\n",e.event,c->GPRx);halt(0);printf("执行了halt之后");break;//EVENT_YIELD
    case 2:do_syscall(c);break;//EVENT_SYSCALL
    default: panic("Unhandled event ID = %d", e.event);break;
   }
  //返回输入的上下文 c，表示处理事件后的上下文状态。
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
