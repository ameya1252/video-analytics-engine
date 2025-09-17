#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace log {
namespace {
std::mutex mtx;
std::string now() {
    using namespace std::chrono;
    auto t = system_clock::to_time_t(system_clock::now());
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
}

void info(const std::string& msg) {
    std::lock_guard<std::mutex> lk(mtx);
    std::cerr << "[INFO  " << now() << "] " << msg << std::endl;
}
void warn(const std::string& msg) {
    std::lock_guard<std::mutex> lk(mtx);
    std::cerr << "[WARN  " << now() << "] " << msg << std::endl;
}
void error(const std::string& msg) {
    std::lock_guard<std::mutex> lk(mtx);
    std::cerr << "[ERROR " << now() << "] " << msg << std::endl;
}
} // namespace log
