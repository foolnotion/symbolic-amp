#pragma once

#include <random>

typedef std::mt19937 engine_type;

// implementation
class random {
public:
  random() {
    std::random_device rd;
    twister_ = engine_type(rd());
  }
    int next(int end) {
        if (end == 0) return 0;
        std::uniform_int_distribution<int> dist(0, end);
        return dist(twister_);
    }

    int next(int begin, int end) {
        if (begin == end) return begin;
        std::uniform_int_distribution<int> dist(begin, end);
        return dist(twister_);
    }

    double next_double() {
        std::uniform_real_distribution<double> real(0,1);
        return real(twister_);
    }

    double next_double(double end) {
        if (end == 0) return 0;
        std::uniform_real_distribution<double> real(0, end);
        return real(twister_);
    }

    double next_double(double begin, double end) {
        if (begin == end) return begin;
        std::uniform_real_distribution<double> real(begin, end);
        return real(twister_);
    }

    void seed(engine_type::result_type s) { twister_.seed(s); }
    const engine_type & engine() const { return twister_; }

private:
    engine_type twister_;
};