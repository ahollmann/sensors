#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

struct ReadOnlyPrintWindow {
  std::size_t begin;
  std::size_t end;
  std::reference_wrapper<std::vector<double> const> ref;
};
class Sensor {
 public:
  Sensor(std::uint64_t sampling_rate, std::uint64_t sampling_window,
         std::string name, std::string label, double min_value,
         double max_value, std::string path)
      : sampling_rate_(sampling_rate),
        sampling_window_(sampling_window),
        samples_in_window_(sampling_rate_ * sampling_window_),
        sampling_buffer_size_(2 * samples_in_window_),
        index_next_element_(0),
        name_(std::move(name)),
        label_(std::move(label)),
        min_value_(min_value),
        max_value_(max_value),
        path_(std::move(path)) {
    samples_ = std::vector<double>(sampling_buffer_size_, {});
  };

  double read_sample();

  [[nodiscard]] std::string const& getLabel() const { return label_; };
  [[nodiscard]] std::string const& getName() const { return name_; };
  [[nodiscard]] double getMin() const { return min_value_; };
  [[nodiscard]] double getMax() const { return max_value_; };
  [[nodiscard]] std::uint64_t getSamplingRate() const {
    return sampling_rate_;
  };
  [[nodiscard]] std::uint64_t getSamplingWindow() const {
    return sampling_window_;
  };
  [[nodiscard]] ReadOnlyPrintWindow getPrintWindow() const {
    return {index_next_element_ + samples_in_window_ - valid_elements_,
            index_next_element_ + samples_in_window_, samples_};
  };

 private:
  std::uint64_t const sampling_rate_;    // frequency in HZ
  std::uint64_t const sampling_window_;  // time in seconds
  std::uint64_t const
      samples_in_window_;  // number of samples within sample windows

  // sample buffer size (2 * sampling_rate * sampling_window)
  std::size_t const sampling_buffer_size_;

  std::string const name_;
  std::string const label_;

  double const min_value_;
  double const max_value_;

  std::string const path_;

  // index to the next element
  std::size_t index_next_element_;

  std::size_t valid_elements_{};

  // Type double is used. We waste some space, but double is most versatile
  // and can store singed integers with a width of 52 bit exactly
  // We might need type double later.
  std::vector<double> samples_;

  // Can be extended with timestamps if necessary
  // std::vector<std::chrono::time_point<std::chrono::steady_clock>>
  // timestamps_;
};