#include <common.h>
/*EVENT_NULL = 0,
    EVENT_YIELD, EVENT_SYSCALL, EVENT_PAGEFAULT, EVENT_ERROR,
    EVENT_IRQ_TIMER, EVENT_IRQ_IODEV,*/
static Context* do_event(Event e, Context* c) {
 
   switch (e.event) {

    case EVENT_YIELD:
        Log("schedule之前%x\n",c->mepc);
        c = schedule(c);
        Log("schedule之后 %x\n",c->mepc);
        break;
    case EVENT_SYSCALL:do_syscall(c);break;//EVENT_SYSCALL
    default: panic("Unhandled event ID = %d", e.event);break;
   }
  //返回输入的上下文 c，表示处理事件后的上下文状态。
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
