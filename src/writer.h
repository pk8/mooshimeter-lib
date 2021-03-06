#ifndef __WRITER_H
#define __WRITER_H

#include <string>
#include <future>
#include <chrono>
#include "ble.h"
#include "config_tree.h"
#include "response.h"

class Writer
{
 private:
  BLE& ble;
  ConfigTree& config;
  ExpectedResponses& expected;
  std::atomic<std::chrono::time_point<std::chrono::steady_clock>>
    next_heartbeat;
  struct E
  {
    std::string cmd;
    std::string buf;
    std::promise<std::string> p;
  };
  BlockingQueue<E> q;

  std::atomic<bool> running;
  std::thread writer_t;
  void writer_thread();

  std::condition_variable cv;
  std::thread heartbeat_t;
  void heartbeat_thread();
 public:
  Writer(BLE& _ble, ConfigTree& _config, ExpectedResponses& _expected);
  ~Writer();
  std::future<std::string> cmd(std::string cmd);
};

#endif // __WRITER_H
