# [WIP] Make A Memory Pool

This is a WIP Repo for making a memory pool
这是一个正在建设的内存池手搓小计划课程

跟随的B站网址在：
1. [从什么是内存池出发](https://www.bilibili.com/video/BV14g6TBcEyL/)
2. [native malloc什么时候，哪里不行了？（上）](https://www.bilibili.com/video/BV1ES6hBQE1E)
3. [native malloc什么时候，哪里不行了？（下）](https://www.bilibili.com/video/BV1hN6aB6EU9)
（2，3部分的仓库源码在native_malloc_test下，点击即可查看所有测试代码）
4. MyMemoryPool下是我们做的内存池:
```bash
# 请使用WSL先，我在WSL开发的，不知道Windows能不能用，我没测试过哈
cd MyMemoryPool
cmake -B build -S .
cd build
make -j4 # 多大线程nproc
# 然后运行生成的可执行文件，在终端看效果即可
```






