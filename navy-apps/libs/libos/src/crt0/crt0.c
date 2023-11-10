#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
   // 从参数中获取argc的值
  int argc = (int)args[0];
  
  // 从参数中获取argv的地址
  char **argv = (char **)(args + 1);

  // 从参数中获取envp的地址
  char **envp = (char **)(args + argc + 1);

  // 设置environ为envp
  environ = envp;

  // 调用main函数
  exit(main(argc, argv, envp));
  assert(0);
}
