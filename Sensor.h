#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

class Sensor {
 public:
  Sensor(std::uint64_t sampling_rate, std::uint64_t sampling_window,
         std::string name, std::string label, double min_value,
         double max_value, std::string path)
      : sampling_rate_(sampling_rate),
        sampling_window_(sampling_window),
        samples_in_window_(sampling_rate_ * sampling_window_),
        sampling_buffer_size_(2 * samples_in_window_),
        index_next_element_(samples_in_window_),
        name_(std::move(name)),
        label_(std::move(label)),
        min_value_(min_value),
        max_value_(max_value),
        path_(std::move(path)) {
    samples_ = std::vector<double>(sampling_buffer_size_, {});
  };

  double read_sample();

 private:
  std::uint64_t const sampling_rate_;      // frequency in HZ
  std::uint64_t const sampling_window_;    // time in seconds
  std::uint64_t const samples_in_window_;  // time in seconds

  // sample buffer size (2 * sampling_rate * sampling_window)
  std::size_t const sampling_buffer_size_;
  // index to the next element
  std::size_t index_next_element_;

  std::string name_;
  std::string label_;

  double min_value_;
  double max_value_;

  std::string path_;

  // Type double is used. We waste some space, but double is most versatile
  // and can store singed integers with a width of 52 bit exactly
  // We might need it later.
  std::vector<double> samples_;

  // Can be extended with timestamps if necessary
  // std::vector<std::chrono::time_point<std::chrono::steady_clock>>
  // timestamps_;
};