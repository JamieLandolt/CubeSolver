#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <vector>
#include <unordered_map>
#include <numeric>

class Timer {
private:
        std::chrono::high_resolution_clock::time_point start_time;
        std::unordered_map<int,std::vector<std::chrono::milliseconds>> times;
public:
        void start() {
                start_time = std::chrono::high_resolution_clock::now();
        }

        auto stop(int scramble_size) {
                std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                        std::chrono::high_resolution_clock::now() - start_time);
                times[scramble_size].push_back(time);
                return time;

        }

        std::unordered_map<int,std::vector<std::chrono::milliseconds>>& get_times() {
                return times;
        }

        long avg(std::vector<std::chrono::milliseconds> ts) {
                std::chrono::milliseconds base{0};
                std::chrono::milliseconds total = std::accumulate(ts.begin(), ts.end(), base);
                return (total / ts.size()).count();
        }

        std::unordered_map<int,long> get_avg_times() {
                std::unordered_map<int,long> avg_times;
                for (const auto& [size, times] : times) {
                        avg_times[size] = avg(times);
                }
                return avg_times;
        }
};

#endif
