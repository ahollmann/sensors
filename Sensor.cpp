#include "Sensor.h"

#include <utility>

void Sensor::add_sample(double sample) {
  // store sample in first half of the sample buffer
  samples_[index_next_element_] = sample;

  // store sample in second half of the sample buffer
  samples_[samples_in_window_ + index_next_element_] = sample;

  // adjust our index - stay inside our sample window size
  index_next_element_ = (index_next_element_ + 1) % samples_in_window_;
}