#include <am.h>
#include <nemu.h>
#include <stdint.h>  // 用于 uint32_t 类型



void __am_timer_init() {
  outl(RTC_ADDR, 0);
  outl(RTC_ADDR + 4, 0);
}

/*
回调函数通过 get_time() 获取启动后的时间，并存放在地址 rtc_port_base 处。
__am_timer_uptime 的实现通过向 uptime->us 赋值的方式更新抽象寄存器中保存的时间。
io_read 读取抽象寄存器中保存的时间。
*/
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = inl(RTC_ADDR + 4);
  uptime->us <<= 32;
  uptime->us += inl(RTC_ADDR);
}


void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
