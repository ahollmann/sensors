#include <stdio.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>

#include "Sensor.h"
#include "json.hpp"

void usage(char** argv) {
  std::clog << "Application: " << argv[0]
            << " requires the path to the config file." << std::endl;
}

struct SensorPeriodicTimer {
  std::chrono::time_point<std::chrono::steady_clock> wakeup_time;
  std::chrono::nanoseconds increment_time;
  std::size_t sensor_index;
};

void create_gnuplot_persistent_process_config(std::uint64_t sampling_window,
                                       std::vector<Sensor> const& sensors) {
  std::string gnuplot_config;

  gnuplot_config.append(R"(
set title "Plotting Sensor Data over Time"
set xdata
set xlabel "Time from now in seconds"
set format x "+%.3f"
)");
  std::string xrange;
  xrange += "set xrange [" + std::to_string(sampling_window) + ":0]\n";
  gnuplot_config.append(xrange);

  std::size_t y_count{1};

  std::string filenames("filenames = \"");

  for (auto const& s : sensors) {
    std::string const y_count_string =
        y_count > 1 ? std::to_string(y_count) : "";
    std::string y_string;

    y_string += "set y" + y_count_string + "label \"" + s.getLabel() + "\"\n";
    gnuplot_config.append(y_string);

    y_string.clear();
    y_string += "set y" + y_count_string + "range [" + std::to_string(s.getMin()) + ":" + std::to_string(s.getMax()) +"]\n";
    gnuplot_config.append(y_string);

    auto filename = std::string("/tmp/") + s.getName() + ".dat ";
    filenames += filename;

    // create empty data file -- gnuplot wants one
    std::ofstream out(filename);
    out << 0 << "\t" << 0 << std::endl;
    out.close();

    ++y_count;
  }

  filenames += "\"\n";

  gnuplot_config.append(filenames);

  gnuplot_config.append(
      "plot for [file in filenames] file using 1:2 with lines\n");

  gnuplot_config.append(R"(
while (1) {
	pause 2
	replot
}
)");

  std::clog << gnuplot_config << std::endl;

  std::ofstream out("/tmp/gnuplot_config.txt");
  out << gnuplot_config;
  out.close();
}

int start_gnuplot_persistent_process() {
  std::vector<char*> args;
  args.push_back(const_cast<char*>("/usr/bin/gnuplot"));
  args.push_back(const_cast<char*>("-persist"));
  args.push_back(const_cast<char*>("/tmp/gnuplot_config.txt"));
  args.push_back(nullptr);

  pid_t pid = fork();

  if (pid == -1) {
    std::clog << "Error in fork()!" << std::endl;
    ::exit(EXIT_FAILURE);
  } else if (pid == 0) {
    auto rv = ::execv("/usr/bin/gnuplot", args.data());

    if (rv == -1) {
      std::clog << "Error in execv()!" << std::endl;
      ::exit(EXIT_FAILURE);
    }
    exit(EXIT_FAILURE);  // exec never returns
  }

  return pid;
}

void generate_gnuplot_data(std::vector<Sensor> const& sensors) {
  for (auto const& s : sensors) {
    auto a = s.getPrintWindow();

    std::vector<double> const& v = a.ref.get();

    std::string filename = std::string("/tmp/") + s.getName() + ".dat";
    std::string filename_tmp = std::string("/tmp/") + s.getName() + ".tmp";

    std::ofstream out(filename_tmp);

    double time{};
    double increment_time{1.0 / s.getSamplingRate()};

    // std::clog << "begin_idx: " << a.begin << " end_idx: " << a.end <<
    // std::endl;

    // for (std::size_t i{a.end - 1}; i >= a.begin; --i) {
    //   std::clog << v[i] << "\t" << time << std::endl;
    //   time += increment_time;
    // }

    for (std::size_t i{a.end - 1}; i > a.begin; --i) {
      out << time << "\t" << v[i] << std::endl;
      time += increment_time;
    }

    out.close();

    rename(filename_tmp.c_str(), filename.c_str());
  }
}

int main(int argc, char** argv) {
  using namespace std::chrono_literals;

  if (argc < 2) {
    usage(argv);
    exit(EXIT_FAILURE);
  }

  std::ifstream config_file(argv[1]);

  if (!config_file.good()) {
    std::clog << "File: " << argv[1] << " cannot be accessed." << std::endl;
    exit(EXIT_FAILURE);
  }

  nlohmann::json config_json;
  config_file >> config_json;

  std::vector<Sensor> sensors;
  sensors.reserve(config_json["sensors"].size());

  std::vector<SensorPeriodicTimer> timers;
  sensors.reserve(config_json["sensors"].size());

  std::uint64_t const sampling_window{config_json["sampling_window"]};

  std::clog << config_json.dump(4) << std::endl;

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

  create_gnuplot_persistent_process_config(sampling_window, sensors);

  auto gnu_plot_pid = start_gnuplot_persistent_process();

  // Main loop for reading sensors --- Only read_sample() should be called here to meet the timing requirements
  while (true) {
    std::this_thread::sleep_until(timers.front().wakeup_time);

    timers.front().wakeup_time += timers.front().increment_time;

    auto d = sensors[timers.front().sensor_index].read_sample();

    std::clog << "Read: " << d << std::endl;

    std::make_heap(std::begin(timers), std::end(timers),
                   [](SensorPeriodicTimer& lhs, SensorPeriodicTimer& rhs) {
                     return lhs.wakeup_time > rhs.wakeup_time;
                   });
    // Time is running out ...

    // This belongs into a seperate thread and called periodically, access to sensors next_element needs
    // to be protected. Access to the samples is safe without a mutex if plotting takes less time then
    // then the time in sampling_widow. We will never access the same elements.
    generate_gnuplot_data(sensors);

    // The same guarantees of our storage pattern can be use for storing the data on disc.
    //A separate thread should be used, since IO can be blocking
  }

  return 1;
}