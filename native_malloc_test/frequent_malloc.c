#include <stdlib.h>
int main() {
  // 小块：触发brk
  for (int i = 0; i < 100; i++)
    malloc(1024);

  // 大块：触发mmap
  for (int i = 0; i < 10; i++)
    malloc(200 * 1024);

  return 0;
}