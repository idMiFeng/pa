#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);//调用用户提供的页面分配函数 pgalloc_f 来为 kas 分配一个页4kb大小的内存，把物理页首地址即页目录地址赋值给ptr

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));//调用pgalloc_usr()返回分配的物理页的首地址
  as->ptr = updir; //物理页首地址也就是页目录地址赋值给ptr
  as->area = USER_SPACE;  //虚拟内存0x40000000 ~ 0x80000000,虚拟地址空间中用户态的范围，每个进程都会访问这一范围虚拟地址
  as->pgsize = PGSIZE;//页面的大小4kb
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);//将内核地址空间的内容复制到新分配的用户页目录
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}
/*#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10
#define PTE_A 0x40
#define PTE_D 0x80*/
#define VA_OFFSET(addr) (addr & 0x00000FFF) //提取虚拟地址的低 12 位，即在页面内的偏移。
#define VA_VPN_0(addr)  ((addr >> 12) & 0x000003FF) //提取虚拟地址的中间 10 位，即二级页号
#define VA_VPN_1(addr)  ((addr >> 22) & 0x000003FF) //提取虚拟地址的高 10 位，即一级页号

#define PA_OFFSET(addr) (addr & 0x00000FFF)//提取物理地址的低 12 位，即在页面内的偏移
#define PA_PPN(addr)    ((addr >> 12) & 0x000FFFFF)//提取物理地址的高 20 位，即物理页号
#define PTE_PPN 0xFFFFF000 // 31 ~ 12
void map(AddrSpace *as, void *va, void *pa, int prot) {

  uintptr_t va_trans = (uintptr_t) va;
  uintptr_t pa_trans = (uintptr_t) pa;

  assert(PA_OFFSET(pa_trans) == 0);
  assert(VA_OFFSET(va_trans) == 0);
 
  //提取虚拟地址的二级页号和一级页号，以及物理地址的物理页号
  uint32_t ppn = PA_PPN(pa_trans);
  uint32_t vpn_1 = VA_VPN_1(va_trans);
  uint32_t vpn_0 = VA_VPN_0(va_trans);

  //获取地址空间的页表基址和一级页表的目标位置
  PTE * page_dir_base = (PTE *) as->ptr;
  PTE * page_dir_target = page_dir_base + vpn_1;
  
  //如果一级页表中的页表项的地址(二级页表的基地址)为空，创建并填写页表项
  if (*page_dir_target == 0) { 
    //通过 pgalloc_usr 分配一页物理内存，作为二级页表的基地址
    PTE * page_table_base = (PTE *) pgalloc_usr(PGSIZE);
    //将这个基地址填写到一级页表的页表项中，同时设置 PTE_V 表示这个页表项是有效的。
    *page_dir_target = ((PTE) page_table_base) | PTE_V;
    //计算在二级页表中的页表项的地址
    PTE * page_table_target = page_table_base + vpn_0;
    //将物理页号 ppn 左移 12 位，即去掉低 12 位的偏移，与权限标志 PTE_V | PTE_R | PTE_W | PTE_X 组合，填写到二级页表的页表项中。
    *page_table_target = (ppn << 12) | PTE_V | PTE_R | PTE_W | PTE_X;
  } else {
    //取得一级页表项的内容，然后 & PTE_PPN 通过按位与操作提取出页表的基地址，提取高20位，低 12 位为零
    PTE * page_table_base = (PTE *) ((*page_dir_target) & PTE_PPN);
    //通过加上 vpn_0 计算得到在二级页表中的目标项的地址
    PTE * page_table_target = page_table_base + vpn_0;
    //将物理页号 ppn 左移 12 位，即去掉低 12 位的偏移，与权限标志 PTE_V | PTE_R | PTE_W | PTE_X 组合，填写到二级页表的目标项中。
    *page_table_target = (ppn << 12) | PTE_V | PTE_R | PTE_W | PTE_X;
  }

}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *uctx = (Context *)(kstack.end-sizeof(Context));
  uctx->mepc=(uintptr_t) entry;
  return uctx;
}
