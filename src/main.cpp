//
//  Created by Yoshinori Suzuki on 2026
//
#include <dispatch/dispatch.h>
#include <format>
#include <iostream>
#include <unistd.h>

//
template <class... Args>
void Print(std::format_string<Args...> format, Args... args) {
  std::cout << "\x1b[33m" << std::format(format, std::forward<Args>(args)...)
            << "\x1b[0m\n";
}

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
int main() {
  Print("Hello, C++20!");

  Semaphore sem{5};
  Group group{"com.example.gcdtest"};

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
