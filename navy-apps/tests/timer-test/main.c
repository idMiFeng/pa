#include <stdio.h>
#include <assert.h>
#include <sys/time.h>

int main() {
  struct timeval start, end;
  gettimeofday(&start, NULL);
  while (1) {
    gettimeofday(&end, NULL);
    // 超过0.5秒打印一次
    if (end.tv_sec - start.tv_sec >= 0 && end.tv_usec - start.tv_usec >= 500000) {
      printf("Hello world!\n");
      start = end;
    }
  }
  return 0;
}