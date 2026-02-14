#pragma once

// 1. FreeList
// [meta data] -> [] -> []
// [data] -> [] -> []
#include "memory_pool_config.h"
#include <cstddef>
#include <mutex>
#include <new>
#include <utility>
struct FreeNode {
  FreeNode *next;
};

class FreeList {
public:
  // push
  // [] -> [] -> [] -> [] -> []
  void push(FreeNode *node) {
    node->next = head;
    head = node;
  }
  // pop
  FreeNode *pop() {
    if (!head) {
      return head;
    }

    FreeNode *node = head;
    head = node->next;
    return node;
  }

  // empty
  bool empty() const { return head == nullptr; }

private:
  FreeNode *head; // O(1)
};

// malloc
// free

class CentralPool {
public:
  // 找中心池要内存
  FreeNode *fetch(std::size_t size) {
    std::lock_guard<std::mutex> lock(mutex_); // RAII
    auto &list = pool_[idx(size)];
    if (!list.empty()) {
      return list.pop();
    }
    return nullptr;
  }

  void release(size_t size, FreeNode *node) {
    std::lock_guard<std::mutex> lock(mutex_);
    pool_[idx(size)].push(node);
  }

private:
  static constexpr size_t kMaxSmallSize = _kMaxSmallSize; // 128B
  static constexpr size_t kClassGrid = _kClassGrid;
  static constexpr size_t kNumClasses = kMaxSmallSize / kClassGrid;

  FreeList pool_[kNumClasses];
  std::mutex mutex_;

  static size_t idx(size_t sz) {
    size_t aligned = (sz + kClassGrid - 1) / kClassGrid;
    if (aligned == 0) {
      aligned = 1;
    }
    return aligned - 1;
  }
};

class ThreadCache {
public:
  FreeNode *alloc(size_t size, CentralPool &centralPool) {
    auto &list = freelists_[idx(size)];
    if (!list.empty()) {
      return list.pop();
    }

    for (size_t i = 0; i < _kFetchTime; i++) {
      if (auto n = centralPool.fetch(size)) {
        list.push(n);
      } else {
        list.push(systemNewBlock(size));
      }
    }

    return list.pop();
  }

  void free(size_t size, FreeNode *node) { freelists_[idx(size)].push(node); }

private:
  static constexpr size_t kMaxSmallSize = _kMaxSmallSize; // 128B
  static constexpr size_t kClassGrid = _kClassGrid;
  static constexpr size_t kNumClasses = kMaxSmallSize / kClassGrid;

  FreeList freelists_[kNumClasses];
  std::mutex mutex_;

  static size_t idx(size_t sz) {
    size_t aligned = (sz + kClassGrid - 1) / kClassGrid;
    if (aligned == 0) {
      aligned = 1;
    }
    return aligned - 1;
  }

  static FreeNode *systemNewBlock(size_t size) {
    void *p = ::operator new(size);
    return reinterpret_cast<FreeNode *>(p);
  }
};

class MemoryPool {
public:
  static inline constexpr size_t alignGrid(size_t n) {
    return (n + _kClassGrid - 1) & ~size_t(_kClassGrid - 1);
  }

  static void *alloc(size_t size) {
    size = alignGrid(size);

    if (size > _kMaxSmallSize) {
      return ::operator new(size);
    }

    return tls_cache().alloc(size, central_pool);
  }

  static void free(void *ptr, size_t size) {
    if (!ptr) {
      return;
    }

    size = alignGrid(size);

    if (size > _kMaxSmallSize) {
      ::operator delete(ptr);
      return;
    }

    tls_cache().free(size, reinterpret_cast<FreeNode *>(ptr));
  }

  // obj
  template <typename T, typename... Args> static T *make(Args &&...args) {
    void *mem = alloc(sizeof(T));
    try {
      return new (mem) T(std::forward<Args>(args)...);
    } catch (...) {
      free(mem, sizeof(T));
      throw;
    }
  }

  template <typename T> static void destory(T *obj) {
    if (!obj)
      return;
    obj->~T();
    free(reinterpret_cast<void *>(obj), sizeof(T));
  }

private:
  static ThreadCache &tls_cache() {
    thread_local ThreadCache cache;
    return cache;
  }

  static CentralPool central_pool;
};

inline CentralPool MemoryPool::central_pool;

template <typename T> struct PoolAllocator {
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  template <typename U> struct rebind {
    using other = PoolAllocator<U>;
  };

  PoolAllocator() noexcept {}
  template <typename U> PoolAllocator(const PoolAllocator<U> &) noexcept {}

  T *allocate(std::size_t n) {
    void *p = MemoryPool::alloc(n * sizeof(T));
    if (!p) {
      throw std::bad_alloc();
    }
    return static_cast<T *>(p);
  }

  void deallocate(T *p, std::size_t n) noexcept {
    MemoryPool::free(p, n * sizeof(T));
  }

  template <typename U>
  bool operator==(const PoolAllocator<U> &) const noexcept {
    return true;
  }
  template <typename U>
  bool operator!=(const PoolAllocator<U> &) const noexcept {
    return false;
  }
};
