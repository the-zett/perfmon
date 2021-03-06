#include <gtest/gtest.h>

#include "public/perfmon.h"
#include <atomic>
#include <thread>

namespace {

std::atomic<int> global_gcd_1_result(0);
std::atomic<int> global_gcd_2_result(0);
std::atomic<int> global_gcd_4_result(0);

int N = 2 * 1024;

int gcd(int a, int b) {
  for (;;) {
    if (a == 0) {
      return b;
    }
    b %= a;
    if (b == 0) {
      return a;
    }
    a %= b;
  }
}

void gcd_1_thread() {
  for (int t = 0; t < 4; ++t) {
    PERFMON_SCOPE("gcd_1_thread");
    int result = 0;
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        PERFMON_SCOPE("congestion_point");
        result += gcd(i, j);
      }
    }
    global_gcd_1_result += result;
  }
}

void gcd_2_thread() {
  for (int t = 0; t < 2; ++t) {
    PERFMON_SCOPE("gcd_2_thread");
    int result = 0;
    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        PERFMON_SCOPE("congestion_point");
        result += gcd(i, j);
      }
    }
    global_gcd_2_result += result;
  }
}

void gcd_4_thread() {
  PERFMON_SCOPE("gcd_4_thread");
  int result = 0;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      PERFMON_SCOPE("congestion_point");
      result += gcd(i, j);
    }
  }
  global_gcd_4_result += result;
}

}  // namespace

TEST(Performance, Gcd) {
  {  // 1 thread
    std::thread t1(gcd_1_thread);
    t1.join();
  }

  {  // 2 threads
    std::thread t1(gcd_2_thread);
    std::thread t2(gcd_2_thread);
    t1.join();
    t2.join();
  }
  {  // 4 threads
    std::thread t1(gcd_4_thread);
    std::thread t2(gcd_4_thread);
    std::thread t3(gcd_4_thread);
    std::thread t4(gcd_4_thread);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
  }

  EXPECT_EQ(global_gcd_1_result, global_gcd_2_result);
  EXPECT_EQ(global_gcd_1_result, global_gcd_4_result);

  const auto counters = PERFMON_COUNTERS();
  EXPECT_EQ(counters["gcd_1_thread"].Calls(), counters["gcd_2_thread"].Calls());
  EXPECT_EQ(counters["gcd_1_thread"].Calls(), counters["gcd_4_thread"].Calls());

  EXPECT_GT(counters["gcd_1_thread"].Ticks(),
            0.8 * counters["gcd_2_thread"].Ticks());
  EXPECT_GT(counters["gcd_2_thread"].Ticks(),
            0.8 * counters["gcd_4_thread"].Ticks());
}
