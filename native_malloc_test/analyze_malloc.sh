#!/bin/bash

echo "=== 用perf看malloc到底慢在哪里 ==="
echo ""

# 编译
gcc -g -O0 malloc_vs_pool.c -o malloc_vs_pool
gcc -g -O0 -pthread malloc_multithread.c -o malloc_multithread
gcc -g -O0 malloc_overhead.c -o malloc_overhead
echo "[1] Malloc和最最简单的线程池对比（malloc_vs_pool.c）"
echo "================================"
./malloc_vs_pool
echo ""

read -p "按Enter继续多线程测试..."
echo ""

echo "[2] 多线程竞争（malloc_multithread.c）"
echo "================================"
./malloc_multithread
echo ""


read -p "按Enter看看模式方式差异(malloc_overhead.c)"
echo ""
./malloc_overhead
echo ""

read -p "按Enter用strace看系统调用..."
echo ""

echo "[4] strace - 看频繁malloc的系统调用（frequent_malloc.c）"
echo "================================"

cat > /tmp/frequent_malloc.c << 'EOF'
#include <stdlib.h>
int main() {
    // 小块：触发brk
    for (int i = 0; i < 100; i++)
        malloc(1024);
    
    // 大块：触发mmap
    for (int i = 0; i < 10; i++)
        malloc(200*1024);
    
    return 0;
}
EOF
gcc /tmp/frequent_malloc.c -o /tmp/frequent_malloc

echo "小块分配的系统调用："
strace -e brk -c /tmp/frequent_malloc 2>&1 | tail -5

echo ""
echo "大块分配的系统调用："
strace -e mmap -c /tmp/frequent_malloc 2>&1 | tail -5