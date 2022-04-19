/*
 * Copyright (C) 2022-2022 MengX done
 *
 * Author: 1542978973@qq.com
 * Created by HJ on 2022/04/17
 */

#include <cstddef>

namespace MengX {
template <typename T, size_t BlockSize = 4096>
class MemoryPool {
 public:
  using value_type = T;
  using ptr = T*;
  using ref = T&;
  using const_ptr = const T*;
  using const_ref = const T&;
  using size_type = size_t;

  template <typename U>
  struct rebind {
    using other = MemoryPool<U>;
  };

  MemoryPool() noexcept;
  explicit MemoryPool(const MemoryPool& pool) noexcept;
  explicit(MemoryPool&& MemoryPool) noexcept;
  template <typename U>
  explicit MemoryPool(const MemoryPool<U>& pool) noexcept;

  ~MemoryPool() noexcept;

  MemoryPool& operator=(const MemoryPool& pool) = delete;
  MemoryPool& operator=(MemoryPool&& pool) noexcept;

  ptr Allocate();
  void DeAllocate(ptr p);

  template <class U, class... Args>
  void Construct(U* p, Args&&... args);

  template <class U>
  void Destroy(U* p);

  template <class... Args>
  ptr NewElement(Arg&&... args);
  void DelElement(ptr p);

  PosPtr GetCurrentBlock();
  PosPtr GetCurrentPos();
  PosPtr GetLastPos();
  PosPtr GetFreePos();
  void Reset();

 private:
  struct Pos {
    Pos* next;
  };
  using DataPtr = char*;
  using PosPtr = Pos*;

  void AllocateBlock();
  size_type PadPointer(DataPtr p, size_type align) const noexcept;

 private:
  PosPtr current_block_;
  PosPtr current_pos_;
  PosPtr last_pos_;
  PosPtr free_pos_;
};

template <typename T, size_t BlockSize>
inline PosPtr MemoryPool<T, BlockSize>::CurrentBlock() {
  return current_block_;
}

template <typename T, size_t BlockSize>
inline PosPtr MemoryPool<T, BlockSize>::CurrentPos() {
  return current_pos_;
}
template <typename T, size_t BlockSize>
inline PosPtr MemoryPool<T, BlockSize>::LastPos() {
  return last_pos_;
}
template <typename T, size_t BlockSize>
inline PosPtr MemoryPool<T, BlockSize>::FreePos() {
  return free_pos_;
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::Reset() {
  current_block_ = nullptr;
  current_pos_ = nullptr;
  last_pos_ = nullptr;
  free_pos_ = nullptr;
}

}  // namespace MengX
