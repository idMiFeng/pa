#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

int sys_execve(const char *fname, char *const argv[], char *const envp[]) {
    context_uload(current, fname, argv, envp);
    switch_boot_pcb();
    yield();
    return -1;
}


void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
   Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (const char *)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  Area stack;
  stack.start = pcb->stack;
  stack.end = pcb->stack + STACK_SIZE;
  pcb->cp = kcontext(stack, entry, arg);
  /*Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *kctx = (Context *)(kstack.end-sizeof(Context));
  kctx->mepc=(uintptr_t) entry;
  kctx->GPR2 = (uintptr_t)arg;
  return kctx;
}
*/
}

           //写在loader.c中
// void context_uload(PCB *pcb, const char *filename) {
//   Area stack;
//   stack.end=heap.end;
//   stack.start = stack.end- STACK_SIZE;
//   uintptr_t entry = loader(pcb, filename);

//   pcb->cp = ucontext(NULL, stack, (void*)entry);
//   pcb->cp->GPRx = (uintptr_t) heap.end;
//   /*Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
//   Context *uctx = (Context *)(kstack.end-sizeof(Context));
//   uctx->mepc=(uintptr_t) entry;
//   return uctx;
// }*/
// }

void init_proc() {
  //context_kload(&pcb[0], hello_fun, "A");
  //context_uload(&pcb[1],"/bin/menu");
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  const char filename[] = "/bin/bmp-test";
  naive_uload(NULL, filename);
}

Context* schedule(Context *prev) {
  // save the context pointer
 
  current->cp = prev;
 
// always select pcb[0] as the new process
  current = ((current == &pcb[0]) ? &pcb[1] : &pcb[0]);
  
// then return the new context
  return current->cp;
}
