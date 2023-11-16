#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  pf += nr_page * PGSIZE;
  
  return pf;
}

#ifdef HAS_VME
//pg_alloc()的参数是分配空间的字节数
static void* pg_alloc(int n) {
  int nr_page = n / PGSIZE;
  assert(nr_page * PGSIZE == n);
  void *end = new_page(nr_page);
  void *start = end - n;
  memset(start, 0, n);
  return start;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  /*ROUNDUP(heap.start, PGSIZE) 的具体过程：

如果 heap.start 已经是页面大小的倍数，那么它会保持不变，因为向上舍入到最接近的页面大小的倍数就是它本身。

如果 heap.start 不是页面大小的倍数，那么 ROUNDUP 就会找到比 heap.start 大且是页面大小的倍数的下一个地址。这个操作通常涉及一些位运算，确保结果是页面大小的倍数。*/
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
