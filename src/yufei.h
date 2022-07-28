#include <atomic>
#include <stdlib.h>
#include <thread>

template <class T>
class MpscQueue {
public:
  MpscQueue(int size) : arr_size_(size) {
    arr_ = (T**)calloc(arr_size_, sizeof(*arr_));
    written_ = (bool*)calloc(arr_size_, sizeof(*written_));
  }
  ~MpscQueue() {}

  void Push(T* v) {
    uint64_t tail = std::atomic_fetch_add(&tail_, (uint64_t)1);
    uint64_t idx = tail % arr_size_;
    while (tail - head_ >= arr_size_) {
      std::this_thread::yield();
    }

    arr_[idx] = v;
    std::atomic_signal_fence(std::memory_order_seq_cst);
    written_[idx] = true;
  }

  T* Pop() {
    uint64_t idx = head_ % arr_size_;
    while (!written_[idx]) {
      std::atomic_signal_fence(std::memory_order_relaxed);
    }
    T* res = arr_[idx];
    written_[idx] = false;
    std::atomic_signal_fence(std::memory_order_seq_cst);
    head_++;
    return res;
  }

private:
  uint64_t arr_size_ = 0;
  T** arr_ = nullptr;
  bool* written_ = nullptr;
  uint64_t head_ = 0;
  std::atomic<uint64_t> tail_{0};
};
