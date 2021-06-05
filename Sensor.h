#include <cstdint>
#include <string>
#include <vector>
#include <chrono>

class Sensor
{
public:
    Sensor(std::uint64_t sampling_rate,
           std::uint64_t sampling_window,
           std::string name,
           std::string label,
           double min_value,
           double max_value
           ) :     sampling_rate_(sampling_rate), sampling_window_(sampling_window), sampling_buffer_size_(2 * sampling_rate_ * sampling_window_), name_(std::move(name)), label_(std::move(label)),
                   min_value_(min_value), max_value_(max_value) {};

    void add_sample(double sample);

private:
    std::uint64_t sampling_rate_; // frequency in HZ
    std::uint64_t sampling_window_; // time in seconds

    // sample buffer size (2 * sampling_rate * sampling_window)
    std::size_t sampling_buffer_size_;

    std::string name_;
    std::string label_;

    double min_value_;
    double max_value_;

    // Type double is used. We waste some space, but double is most versatile
    // and can store singed integers with a width of 52 bit exactly
    // We might need it later.
    std::vector<double> samples_;

    // Can be extended with timestamps if necessary
    // std::vector<std::chrono::time_point<std::chrono::steady_clock>> timestamps_;
};