#include "Sensor.h"

#include <fstream>
#include <iostream>
#include <utility>

double Sensor::read_sample() {
  std::ifstream sensor_file(path_);

  if (!sensor_file.good()) {
    std::cout << "Cannot open file: " << path_ << std::endl;
    exit(EXIT_FAILURE);
  }

  double sample{};
  sensor_file >> sample;

  // store sample in first half of the sample buffer
  samples_[index_next_element_] = sample;

  // store sample in second half of the sample buffer
  samples_[samples_in_window_ + index_next_element_] = sample;

  // adjust our index - stay inside our sample window size
  index_next_element_ = (index_next_element_ + 1) % samples_in_window_;

  valid_elements_ = std::min(++valid_elements_, samples_in_window_);

  return sample;
}