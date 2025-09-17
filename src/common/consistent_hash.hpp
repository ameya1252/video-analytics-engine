#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <mutex>

// Very small consistent hashing ring for routing stream_id to an inference backend.
class ConsistentHash {
    std::map<size_t, std::string> ring_;
    size_t replicas_;
    std::hash<std::string> hasher_;
    std::mutex mtx_;
public:
    explicit ConsistentHash(size_t replicas = 100) : replicas_(replicas) {}
    void set_backends(const std::vector<std::string>& backends) {
        std::lock_guard<std::mutex> lk(mtx_);
        ring_.clear();
        for (auto& b : backends) {
            for (size_t i = 0; i < replicas_; ++i) {
                ring_.emplace(hasher_(b + "#" + std::to_string(i)), b);
            }
        }
    }
    std::string route(const std::string& key) const {
        if (ring_.empty()) return {};
        auto h = hasher_(key);
        auto it = ring_.lower_bound(h);
        if (it == ring_.end()) return ring_.begin()->second;
        return it->second;
    }
};
