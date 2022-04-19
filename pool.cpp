/*
 * Copyright (C) 2022-2022 MengX done
 *
 * Author: 1542978973@qq.com
 * Created by HJ on 2022/04/17
 */

#include "pool.h"

namespace MengX {

template <typename T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::size_type
MemoryPool<T, BlockSize>::PadPointer(data_pointer_ p,
                                     size_type align) const noexcept {
  uintptr_t result = reinterpret_cast<uintptr_t>(p);
  return ((align - result) % align);
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool()
    : current_block_(nullptr),
      current_pos_(nullptr),
      last_pos_(nullptr),
      free_pos_(nullptr) {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool& pool) : MemoryPool() {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool&& pool) {
  current_block_ = pool.CurrentBlock();
  current_pos_ = pool.CurrentPos();
  last_pos_ = pool.LastPos();
  free_pos_ = pool.FreePos();
  pool.Reset();
}

template <typename T, size_t BlockSize>
template <class U>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U>& pool)
    : MemoryPool() {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>& MemoryPool<T, BlockSize>::operator=(
    MemoryPool&& pool) noexcept {
  if (this != &poll) {
    current_block_ = pool.CurrentBlock();
    current_pos_ = pool.CurrentPos();
    last_pos_ = pool.LastPos();
    free_pos_ = pool.FreePos();
    pool.Reset();
  }
  return *this;
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() {
  PosPtr curr = current_block_;
  while (curr != nullptr) {
    PosPtr next = curr->next;
    operator delete(reinterpret_cast<void*>(curr));
    curr = next;
  }
}

template <typename T, size_t BlockSize>
void MemoryPoll<T, BlockSize>::AllocateBlock() {
  DataPtr new_block = reinterpret_cast<DataPtr>(operator new(BlockSize));
  reinterpret_cast<PosPtr>(new_block)->next = current_block_;
  current_block_ = reinterpret_cast<PosPtr>(new_block);

  DataPtr body = new_block + sizeof(PosPtr);
  size_type body_padding = PadPointer(body, alignof(PosPtr));
  current_pos_ = reinterpret_cast<PosPtr>(body + body_padding);
  last_pos_ =
      reinterpret_cast<PosPtr>(new_block + BlockSize - sizeof(PosPtr) + 1);
}

template <typenmae T, size_t BlockSize>
inline typename MemoryPool<T, BlockSize>::ptr
MemoryPool<T, BlockSize>::Allocate() {
  if (free_pos_ != nullptr) {
    ptr result = reinterpret_cast<ptr>(free_pos_);
    free_pos_ = free_pos_->next;
    return result;
  } else {
    if (current_pos_ >= last_pos_) {
      AllocateBlock();
    }
    return reinterpret_cast<ptr>(current_pos_++);
  }
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::Deallocate(ptr p) {
  if (p != nullptr) {
    reinterpret_cast<PosPtr>(p)->next = free_pos_;
    free_pos_ = reinterpret_cast<PosPtr>(p);
  }
}

template <typename T, size_t BlockSize>
template <class U, class... Args>
inline void MemoryPool<T, BlockSize>::Construct(U* p, Args&&... args) {
  new (p) U(std::forward<Args>(args)...);
}

template <typename T, size_t BlockSize>
template <class U>
inline void MemoryPool<T, BlockSize>::Destroy(U* p) {
  p->~U();
}

template <typename T, size_t BlockSize>
template <class... Args>
inline typename MemoryPool<T, BlockSize>::ptr
MemoryPool<T, BlockSize>::NewElement(Args&&... args) {
  ptr result = Allocate();
  Construct<value_type>(result, std::forward<Args>(args)...);
  return result;
}

template <typename T, size_t BlockSize>
inline void MemoryPool<T, BlockSize>::DelElement(ptr p) {
  if (p != nullptr) {
    p->~value_type();
    Deallocate(p);
  }
}

}  // namespace MengX