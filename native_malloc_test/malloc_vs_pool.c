#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ITERATIONS 1000000

// 测试1: 频繁malloc/free
double test_malloc_free() {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < ITERATIONS; i++) {
        void *ptr = malloc(64);
        free(ptr);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

// 测试2: 简单内存池
typedef struct {
    char pool[64 * 1000];
    int next;
} SimplePool;

double test_memory_pool() {
    SimplePool pool;
    pool.next = 0;
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    for (int i = 0; i < ITERATIONS; i++) {
        // 从池中"分配"
        char *ptr = &pool.pool[(i % 1000) * 64];
        // 池子不需要free
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main() {
    printf("=== malloc性能瓶颈演示 ===\n\n");
    printf("测试场景: %d次 64字节的分配/释放\n\n", ITERATIONS);
    
    // 预热
    for (int i = 0; i < 1000; i++) {
        void *p = malloc(64);
        free(p);
    }
    
    double malloc_time = test_malloc_free();
    double pool_time = test_memory_pool();
    
    printf("malloc/free方式: %.6f 秒 (%.2f ns/次)\n", 
           malloc_time, malloc_time * 1e9 / ITERATIONS);
    printf("内存池方式:      %.6f 秒 (%.2f ns/次)\n", 
           pool_time, pool_time * 1e9 / ITERATIONS);
    printf("\n性能提升: %.1f倍\n", malloc_time / pool_time);
    
    printf("\nmalloc慢的原因:\n");
    printf("1. 每次都要搜索合适的内存块\n");
    printf("2. 需要维护复杂的元数据\n");
    printf("3. 多线程需要加锁\n");
    printf("4. 内存碎片化管理开销\n");
    
    return 0;
}
