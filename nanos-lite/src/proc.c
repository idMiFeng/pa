#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  Area stack;
  stack.start = pcb->stack;
  stack.end = pcb->stack + STACK_SIZE;
  pcb->cp = kcontext(stack, entry, arg);
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, NULL);//把 hello_fun()函数的地址作为entry传入
  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  //const char filename[] = "/bin/pal";
  //naive_uload(NULL, filename);
}

Context* schedule(Context *prev) {
  // save the context pointer
  current->cp = prev;

// always select pcb[0] as the new process
  current = &pcb[0];

// then return the new context
  return current->cp;
}
