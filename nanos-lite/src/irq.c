#include <common.h>
/*EVENT_NULL = 0,
    EVENT_YIELD, EVENT_SYSCALL, EVENT_PAGEFAULT, EVENT_ERROR,
    EVENT_IRQ_TIMER, EVENT_IRQ_IODEV,*/
static Context* do_event(Event e, Context* c) {
 
   switch (e.event) {
    
    case EVENT_YIELD:
        c = schedule(c);
        break;
    case EVENT_SYSCALL:
        do_syscall(c);break;
    case EVENT_IRQ_TIMER: 
        c = schedule(c);
        assert(c != NULL);        
        break;
    default: panic("Unhandled event ID = %d", e.event);break;
   }
  //返回输入的上下文 c，表示处理事件后的上下文状态。
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
