#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static char HEX[] = "0123456789ABCDEF";
int printf(const char *fmt, ...) {
  char buffer[2048];
	va_list arg;
	va_start(arg, fmt);
	int ret = vsprintf(buffer, fmt, arg);
	putstr(buffer);
	va_end(arg);
	return ret;

}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, -1, fmt, ap);
}

//接受一个格式化字符串和一组可变参数，然后使用 vsprintf 将格式化后的字符串写入到指定的输出缓冲区 out 中，并返回格式化后的字符串的长度。
int sprintf(char *out, const char *fmt, ...) {
  //va_list 是一个用于处理可变参数的类型，通常用于函数参数个数不确定的情况
  va_list arg;
  //va_start 是一个宏，它用于初始化可变参数列表 arg。在这里，它告诉编译器从 fmt 参数之后的可变参数开始处理
  va_start(arg, fmt);
  //它的参数包括输出字符串的地址 out，格式字符串 fmt，以及可变参数列表 arg。它会将格式化后的字符串写入到 out 中，直到遇到字符串结束符 null ('\0')。
  int ret = vsprintf(out, fmt, arg);
  //va_end(arg)：这个宏用于结束对可变参数列表的处理
  va_end(arg);
  //最后，sprintf 函数返回 vsprintf 的返回值，即格式化后的字符串的长度。这个长度不包括字符串结束符 null ('\0')。
  return ret;

}

int snprintf(char *out, size_t n, const char *fmt, ...) {
   va_list arg;
  va_start(arg, fmt);
  int ret = vsprintf(out, fmt, arg);
  va_end(arg);
  return ret;

}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
   char buffer[128];
  char *txt, cha;
  int num, len;
  unsigned int unum;
  uint32_t pointer;
  int state = 0, i, j;
  for(i = 0, j = 0; fmt[i] != '\0'; i++) {
	switch(state)
	{
		case 0:
			if(fmt[i] != '%') {
				out[j] = fmt[i];
				j++;
			}
			else
				state = 1;
			break;
		case 1:
			switch(fmt[i])
			{
				case 's':
					txt = va_arg(ap, char*);
					for(int k = 0; txt[k] != '\0'; k++)
					{
						out[j] = txt[k];
						j++;
					}
					break;
				case 'd':
					num = va_arg(ap, int);
					if(num == 0)
					{
						out[j] = '0';
						j++;
						break;
					}
					if(num < 0)
					{
						out[j] = '-';
						j++;
						num = -num;
					}
					for(len = 0; num; num /= 10, len++)
						buffer[len] = HEX[num % 10];
					for(int k = len - 1; k >= 0; k--)
					{
						out[j] = buffer[k];
						j++;
					}
					break;
				case 'c':
					cha = (char)va_arg(ap, int);
					out[j] = cha;
					j++;
					break;
				case 'p':
					pointer = va_arg(ap, uint32_t);
					for(len = 0; pointer; pointer /= 16, len++)
						buffer[len] = HEX[pointer % 16];
					for(int k = 0; k < 8 - len; k++)
					{
						out[j] = '0';
						j++;
					}
					for(int k = len - 1; k >= 0; k--)
					{
						out[j] = buffer[k];
						j++;
					}
					break;
				case 'x':
					unum = va_arg(ap, unsigned int);
					if(unum == 0) {
						out[j] = '0';
						j++;
						break;
					}
					for(len = 0; unum; unum >>= 4, len++)
						buffer[len] = HEX[unum & 0xF];
					for(int k = len - 1; k >= 0; k--)
					{
						out[j] = buffer[k];
						j++;
					}
					break;
				default:
					
			}
			state = 0;
			break;
	}
  }
  out[j] = '\0';
  return j;

}

#endif
