#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <thread>

#include "Sensor.h"
#include "json.hpp"

void usage(char** argv) {
  std::cout << "Application: " << argv[0]
            << " requires the path to the config file." << std::endl;
}

struct SensorPeriodicTimer {
  std::chrono::time_point<std::chrono::steady_clock> wakeup_time;
  std::chrono::nanoseconds increment_time;
  std::size_t sensor_index;
};

int main(int argc, char** argv) {
  using namespace std::chrono_literals;

  if (argc < 2) {
    usage(argv);
    exit(EXIT_FAILURE);
  }

  std::ifstream config_file(argv[1]);

  if (!config_file.good()) {
    std::cout << "File: " << argv[1] << " cannot be accessed." << std::endl;
    exit(EXIT_FAILURE);
  }

  nlohmann::json config_json;
  config_file >> config_json;

  std::vector<Sensor> sensors;
  sensors.reserve(config_json["sensors"].size());

  std::vector<SensorPeriodicTimer> timers;
  sensors.reserve(config_json["sensors"].size());

  std::uint64_t const sampling_window{config_json["sampling_window"]};

  std::cout << config_json.dump(4) << std::endl;

  auto current_time = std::chrono::steady_clock::now();

  for (auto& [key, value] : config_json["sensors"].items()) {
    if (value["enabled"]) {
      std::uint64_t sampling_rate{value["sampling_rate"]};

      sensors.emplace_back(value["sampling_rate"], sampling_window,
                           value["name"], value["label"], value["min_value"],
                           value["max_value"], value["path"]);

      timers.push_back(SensorPeriodicTimer{current_time + 100ms,
                                           1000000000ns / sampling_rate,
                                           sensors.size() - 1});
    }
  }

  // for (std::size_t i{}; i < 10; ++i) {
  // Main loop for reading sensors
  while (true) {
    std::this_thread::sleep_until(timers.front().wakeup_time);

    timers.front().wakeup_time += timers.front().increment_time;

    std::cout << "Read: " << sensors[timers.front().sensor_index].read_sample()
              << std::endl;

    std::make_heap(std::begin(timers), std::end(timers),
                   [](SensorPeriodicTimer& lhs, SensorPeriodicTimer& rhs) {
                     return lhs.wakeup_time > rhs.wakeup_time;
                   });
  }

  return 1;
}