#pragma once

#include <chrono>
#include <iostream>

namespace pfp {

template<bool enabled>
class Timer;

template<>
class Timer<true> {
private:
    const char* name_;
    std::chrono::high_resolution_clock::time_point start_time_;

public:
    Timer(const char* name)
        : name_(name) {
            start_time_ = std::chrono::high_resolution_clock::now();
        }

    ~Timer() {
        auto end_time =  std::chrono::high_resolution_clock::now();
        auto final_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_).count();
        std::cerr << name_ << ": " << final_time << " ms" << std::endl;
    }
};

template<>
class Timer<false> {
public:
    Timer(const char* name) {}
};


} // namespace pfp
