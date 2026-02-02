#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define THREADS 4
#define PER_THREAD 250000

// malloc方式 - 有锁竞争
void* thread_malloc(void* arg) {
    for (int i = 0; i < PER_THREAD; i++) {
        void *ptr = malloc(64);
        free(ptr);
    }
    return NULL;
}

// 内存池方式 - 每线程独立池，无竞争
typedef struct {
    char pool[64 * 1000];
} ThreadPool;

void* thread_pool(void* arg) {
    ThreadPool *pool = (ThreadPool*)arg;
    for (int i = 0; i < PER_THREAD; i++) {
        char *ptr = &pool->pool[(i % 1000) * 64];
    }
    return NULL;
}

int main() {
    pthread_t threads[THREADS];
    struct timespec start, end;
    
    printf("=== 多线程场景下的malloc瓶颈 ===\n\n");
    printf("测试: %d个线程，每个分配/释放 %d 次\n\n", THREADS, PER_THREAD);
    
    // 测试malloc
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_malloc, NULL);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double malloc_time = (end.tv_sec - start.tv_sec) + 
                         (end.tv_nsec - start.tv_nsec) / 1e9;
    
    // 测试内存池
    ThreadPool pools[THREADS];
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_pool, &pools[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double pool_time = (end.tv_sec - start.tv_sec) + 
                       (end.tv_nsec - start.tv_nsec) / 1e9;
    
    printf("malloc方式: %.6f 秒\n", malloc_time);
    printf("内存池方式: %.6f 秒\n", pool_time);
    printf("\n性能提升: %.1f倍\n\n", malloc_time / pool_time);
    
    printf("malloc在多线程的问题:\n");
    printf("1. Arena锁竞争 - 多线程抢同一个arena\n");
    printf("2. Cache一致性开销 - 不同CPU核心间同步\n");
    printf("3. 上下文切换 - 等锁时CPU调度\n");
    
    return 0;
}
