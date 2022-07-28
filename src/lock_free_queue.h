#ifndef LOCK_FREE_QUEUE_H_
#define LOCK_FREE_QUEUE_H_
#include <atomic>
#include <thread>
#include <string.h>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <vector>

template <class T>
class LockFreeQueue {
public:
  LockFreeQueue(int queue_size) {
    while (mask_ < queue_size) mask_ *= 2;
    data_arr_ = static_cast<T**>(calloc(mask_, sizeof(T*)));
    written_ = static_cast<bool*>(calloc(mask_, sizeof(bool)));
    mask_--;
    fetch_start = std::chrono::system_clock::now();
  }
  LockFreeQueue(const LockFreeQueue& q) = delete;
  LockFreeQueue operator=(const LockFreeQueue& q) = delete;

  ~LockFreeQueue() {}

  bool Push(T* new_data) {
    int cur_tail = tail_.load();
    while (written_[cur_tail & mask_] || !tail_.compare_exchange_weak(cur_tail, cur_tail + 1)) {
      cur_tail = tail_.load();
      //std::cout << "Tail Stuck: " << written_[cur_tail & mask_] << " " << cur_tail << " " << (cur_tail & mask_) << std::endl;
    }

    data_arr_[cur_tail & mask_] = new_data;
    written_[cur_tail & mask_] = true;
    //std::cout << "write " << *new_data << " at " << (cur_tail & mask_) << std::endl;
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return true;
  }

  T* Pop() {
    int cur_head = head_.load();
    while (!written_[cur_head & mask_] || !head_.compare_exchange_weak(cur_head, cur_head + 1)) {
      cur_head = head_.load();
      //std::cout << "Head Stuck: " << written_[cur_head & mask_] << " " << cur_head << " " << (cur_head & mask_) << std::endl;
      //std::this_thread::sleep_for(std::chrono::milliseconds(100));
      //std::atomic_thread_fence(std::memory_order_seq_cst);
      if (reach_time_limit()) return nullptr;
    }

    fetch_start = std::chrono::system_clock::now();
    T* res = data_arr_[cur_head & mask_];
    written_[cur_head & mask_] = false;
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res;
  }

  inline bool reach_time_limit() {
    if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - fetch_start).count() >= 1000) {
      return true;
    }
    return false;
  }

private:
  std::atomic<int> tail_ = {0};
  std::atomic<int> head_ = {0};
  int mask_ = 2;
  static constexpr uint64_t default_queue_size = 1000;
  T** data_arr_;
  bool* written_;
  std::mutex lock_;
  std::chrono::system_clock::time_point fetch_start;
};
#endif
