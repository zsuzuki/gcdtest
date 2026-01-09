//
//  Created by Yoshinori Suzuki on 2026
//
#include <dispatch/dispatch.h>
#include <format>
#include <iostream>
#include <unistd.h>

namespace {

//
class Semaphore {
  dispatch_semaphore_t sem;

public:
  Semaphore(int num = 1) : sem(dispatch_semaphore_create(num)) {}
  ~Semaphore() { dispatch_release(sem); }

  void lock() { dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER); }
  void unlock() { dispatch_semaphore_signal(sem); }
};

//
template <class... Args>
void Print(std::format_string<Args...> format, Args... args) {
  static Semaphore sem;
  std::string str = std::format(format, std::forward<Args>(args)...);
  sem.lock();
  std::cout << "\x1b[33m" << str << "\x1b[0m\n";
  sem.unlock();
}

//
class Group {
  dispatch_group_t group;
  dispatch_queue_t queue;

public:
  Group(const char *label)
      : group(dispatch_group_create()),
        queue(dispatch_queue_create(label, DISPATCH_QUEUE_CONCURRENT)) {}
  ~Group() {
    dispatch_release(queue);
    dispatch_release(group);
  }

  void async(auto func) {
    dispatch_group_async(group, queue, ^{
      func();
    });
  }

  void notify(auto func) {
    dispatch_group_notify(group, queue, ^{
      func();
    });
  }

  void wait() { dispatch_group_wait(group, DISPATCH_TIME_FOREVER); }
};

//
class Timer {
  dispatch_source_t timer;
  dispatch_semaphore_t sem;

public:
  Timer(const char *label, auto func) : sem(dispatch_semaphore_create(0)) {
    auto queue = dispatch_queue_create(label, 0);
    timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);

    dispatch_source_set_cancel_handler(timer, ^{
      dispatch_semaphore_signal(sem);
      Print("Cancel");
    });

    dispatch_source_set_event_handler(timer, ^{
      if (func()) {
        cancel();
      }
    });
  }
  ~Timer() {
    wait();
    dispatch_release(timer);
    dispatch_release(sem);
  }

  void start(uint64_t interval_ms) {
    dispatch_source_set_timer(timer, DISPATCH_TIME_NOW,
                              interval_ms * NSEC_PER_MSEC, 0);
    dispatch_resume(timer);
  }

  void cancel() { dispatch_source_cancel(timer); }
  void wait() { dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER); }
};

} // namespace

//
int main() {
  Print("Hello, C++20!");

  Semaphore sem{5};
  Group group{"com.example.gcdtest"};

  std::atomic_int counter{0};
  Timer timer{"com.example.gcdtest.timer", [&]() {
                counter++;
                Print("Counter: {}", counter.load());
                return counter.load() >= 7;
              }};
  timer.start(1000);

  for (int i = 0; i < 10; ++i) {
    group.async([i, &sem]() {
      sem.lock();
      Print("Task {} started", i + 1);
      sleep(2 + i % 3);
      Print("Task {} completed", i + 1);
      sem.unlock();
    });
  }
  group.notify([&]() { Print("All tasks completed"); });

  Print("Waiting for tasks to complete...");

  group.wait();

  return 0;
}
