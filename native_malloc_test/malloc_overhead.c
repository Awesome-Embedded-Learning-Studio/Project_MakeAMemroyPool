#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void visualize_malloc_overhead() {
  printf("=== malloc元数据开销可视化 ===\n\n");

  // 分配不同大小，看实际占用
  size_t sizes[] = {16, 32, 64, 128, 256};

  for (int i = 0; i < 5; i++) {
    void *p1 = malloc(sizes[i]);
    void *p2 = malloc(sizes[i]);

    // 计算地址差来推测实际占用
    size_t actual = (char *)p2 - (char *)p1;
    size_t overhead = actual - sizes[i];

    printf("malloc(%3zu) 字节:\n", sizes[i]);
    printf("  实际占用: %zu 字节\n", actual);
    printf("  元数据开销: %zu 字节 (%.1f%%)\n", overhead,
           overhead * 100.0 / sizes[i]);
    printf("\n");

    free(p1);
    free(p2);
  }
}

void compare_access_pattern() {
  printf("=== 内存访问模式对比 ===\n\n");

#define N 10000
  struct timespec start, end;

  // malloc方式：离散分配
  void *ptrs[N];
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < N; i++) {
    ptrs[i] = malloc(64);
  }
  // 访问所有内存
  for (int i = 0; i < N; i++) {
    *(char *)ptrs[i] = 1;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  double malloc_access =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

  for (int i = 0; i < N; i++) {
    free(ptrs[i]);
  }

  // 内存池方式：连续内存
  char *pool = malloc(64 * N);
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (int i = 0; i < N; i++) {
    pool[i * 64] = 1;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  double pool_access =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  free(pool);

  printf("访问 %d 个64字节块:\n", N);
  printf("malloc方式 (离散): %.6f 秒\n", malloc_access);
  printf("内存池方式 (连续): %.6f 秒\n", pool_access);
  printf("速度提升: %.1f倍\n\n", malloc_access / pool_access);
}

int main() {
  visualize_malloc_overhead();
  printf("\n");
  compare_access_pattern();

  return 0;
}
