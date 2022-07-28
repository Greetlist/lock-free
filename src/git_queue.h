#include <vector>
#include <atomic>
#include <chrono>

template <typename T> 
class Queue {
private:
  std::vector<T> buffer;
  std::vector<char> isfree;
  std::atomic<int> in, out;
  int mask = 2;
  std::chrono::system_clock::time_point fetch_start;

 public:
  Queue(int size=1) {
    while (mask < size) mask *= 2;
    buffer = std::vector<T>(mask, 0);
    isfree = std::vector<char>(mask, 1);
    out = in = 0; // start in overflow
    mask--; // 01000000 => 00111111
    fetch_start = std::chrono::system_clock::now();
  } 

  int push(const T &item) {
    int i;
    do {
      i = in;
    } while (!isfree[i & mask] || !in.compare_exchange_weak(i, i + 1));

    buffer[i & mask] = item;
    isfree[i & mask] = 0;
    return i;
  }

  int pop(T &item) {
    int o;
    do {
      o = out;
      if (reach_time_limit()) return -1;
    } while (isfree[o & mask] || !out.compare_exchange_weak(o, o + 1));

    item = buffer[o & mask];
    isfree[o & mask] = 1;
    return o;
  }
private:
  inline bool reach_time_limit() {
    if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - fetch_start).count() > 1500) {
      return true;
    }
    return false;
  }
};
