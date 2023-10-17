#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t screen_wh=inl(VGACTL_ADDR);
  //screen_wh包含了屏幕的宽度和高度信息，其中高16位存储宽度，低16位存储高度
  
  //使用&位操作符和0xffff掩码来提取screen_wh中的低16位，这就是屏幕的高度信息。
  uint32_t h=screen_wh & 0xffff;

  //使用>>位操作符来右移screen_wh中的高16位，将它们移到低16位的位置，这就是屏幕的宽度信息。
  uint32_t w=screen_wh >>16;
  //初始化了一个名为cfg的AM_GPU_CONFIG_T类型的结构体
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x=ctl->x,y=ctl->y,w=ctl->w,h=ctl->h;
  //pixels 是一个指向要绘制的图像数据的指针，其中包含要绘制的像素的颜色信息。
  uint32_t *pixels=ctl->pixels;
  //fb 是一个指向帧缓冲的指针，它表示帧缓冲的起始地址。
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t screen_w =inl(VGACTL_ADDR)>>16;
  /*
  for (int i = y; i < y + h; i++) {：外层循环遍历了要绘制的区域的每一行，从 y 开始，一直到 y + h - 1，也就是要
  绘制的区域的底边。

for (int j = x; j < x + w; j++) {：内层循环遍历了每一行中的每一个像素，从 x 开始，一直到 x + w - 1，也就是要绘制
的区域的右边。

fb[screen_w * i + j] = pixels[w * (i - y) + (j - x)];：在内层循环中，将从 pixels 数组中获取的像素颜色信息
复制到帧缓冲中的相应位置。fb[screen_w * i + j] 表示帧缓冲中的一个像素，pixels[w * (i - y) + (j - x)] 表示
从 pixels 数组中获取的相应颜色信息。这个过程实际上是将图像数据从 pixels 复制到帧缓冲中的指定位置。
  */
  for (int i=y;i<y+h;i++){
    for (int j=x;j<x+w;j++){
      fb[screen_w*i+j]=pixels[w*(i-y)+(j-x)];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1); // 同步帧缓冲内容到屏幕
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
