#include <am.h>
#include <nemu.h>

//在键盘事件的数据结构中，通常会使用位掩码来表示按键的状态,该宏用于检测最高位是否被设置。如果最高位被设置，就表示按键处于按下状态。
#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
    kbd->keycode = inl(KBD_ADDR);
    if (kbd->keycode & KEYDOWN_MASK) {
        kbd->keycode ^= KEYDOWN_MASK;
        kbd->keydown = 1;
    } else {
        kbd->keydown = 0;
    }
}
