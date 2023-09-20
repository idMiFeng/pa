#include <common.h>

#define MAX_IRINGBUF 16


void trace_inst(word_t pc,uint32_t inst);
void display_inst();
void display_pread(paddr_t addr,int len);
void display_pwrite(paddr_t addr,int len,word_t data);
//ItraceNOde记录单条指令pc和内容
typedef struct{
    word_t pc;
    uint32_t inst;
}ItraceNode;

//环形缓冲区
ItraceNode iringbuf[MAX_IRINGBUF];

int p_cur=0;
bool full=false;

//存指令
void trace_inst(word_t pc,uint32_t inst){
    iringbuf[p_cur].pc=pc;
    iringbuf[p_cur].inst=inst;
    p_cur=(p_cur+1)%MAX_IRINGBUF;
    full=full || p_cur==0;
}

//取指令
void display_inst(){
    if(!full && !p_cur){
        return;
    }
     int start = 0;
    int end = full ? MAX_IRINGBUF : p_cur;

    // 遍历环形缓冲区，从 start 到 end，打印存储的指令
    for (int i = start; i < end; i++) {
        word_t pc = iringbuf[i].pc;
        uint32_t inst = iringbuf[i].inst;

        // 在这里你可以按照自己的需求输出指令的信息
        printf("PC: 0x%08x, Instruction: 0x%08x\n", pc, inst);
    }
   
}

//mtracei设施，在paddr_read()调用
void display_pread(paddr_t addr,int len){
    printf("pread at" FMT_PADDR "len=%d\n",addr,len);

}

//mtrace设施
void display_pwrite(paddr_t addr,int len,word_t data){
    printf("pwrite at" FMT_PADDR "len=%d,data=" FMT_WORD "\n",addr,len,data);
}