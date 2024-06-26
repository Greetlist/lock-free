#include "lock_free_queue.h"
#include "lock_free_queue_v2.h"
#include "git_queue.h"
#include "git_queue_2.h"
#include "yufei.h"
#include <vector>
#include <chrono>
#include <ctime>
#include <mutex>
#include <algorithm>
#include <glog/logging.h>
#include <cassert>

int main(int argc, char** argv) {
  auto start_time = std::chrono::system_clock::now();
  //LockFreeQueue<int> q(100);
  LockFreeQueueV2<int> q(1000);
  //Queue<int> q(20000);
  //LockFreeQueueCpp11<int> q(20000);
  //MpscQueue<int> q(50);
  std::mutex lock;
  std::vector<int> res_vec;
  std::unordered_map<int*, int> res_map;

  int produce_thread_num = 5;
  int step_size = 10000000;

  std::vector<std::thread> threads;
  for (int j = 0; j < produce_thread_num; ++j) {
    threads.push_back(std::thread([j, step_size, &q]() {
      for (int i = step_size*j; i < step_size*(j+1); ++i) {
        int* item = new int(i);
        while (!q.Push(item)) {}
      }
      std::cout << "produce exit" << std::endl;
    }));
  }

  auto update_time = std::chrono::system_clock::now();
  for (int i = 0; i < 2; ++i) {
    threads.push_back(std::thread([&]() {
      while (true) {
        int* res = q.Pop();
        if (res) {
          std::lock_guard<std::mutex> lk(lock);
          res_vec.emplace_back(*res);
          update_time = std::chrono::system_clock::now();
        }
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - update_time).count() > 1000) {
          break;
        }
      }
    }));
  }

  for (int i = 0; i < threads.size(); ++i) {
    threads[i].join();
  }

  std::chrono::microseconds cost = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - start_time);
  std::cout << "Total Cost micro seconds is: " 
      << cost.count() / 1000000 << ","
      << (cost.count() / 1000) % 1000 << ","
      << cost.count() % 1000 << std::endl;

  std::vector<int> invalid;
  sort(res_vec.begin(), res_vec.end());
  std::cout << "Size is: " << res_vec.size() << std::endl;
  assert(res_vec.size() == step_size * produce_thread_num);
  for (int i = 0; i < res_vec.size(); ++i) {
    //std::cout << i << " " << res_vec[i] << std::endl;
    CHECK(i == res_vec[i]);
    //if (i != res_vec[i]) {
      //invalid.emplace_back(res_vec[i]);
    //}
  }
  //std::size_t size = invalid.size();
  //if (size != 0) {
    //for (int i = 0; i < size; ++i) {
      //std::cout << invalid[i] << std::endl;
    //}
    //return -1;
  //}
  return 0;
}
