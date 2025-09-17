#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "../common/json.hpp"
#include "../common/logging.hpp"  

namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

using steady_clock_t = std::chrono::steady_clock;

int main() {
    std::string host = std::getenv("GATEWAY_HOST") ? std::getenv("GATEWAY_HOST") : "localhost";
    int port = std::getenv("GATEWAY_PORT") ? std::atoi(std::getenv("GATEWAY_PORT")) : 8080;
    std::string stream_id = std::getenv("STREAM_ID") ? std::getenv("STREAM_ID") : "demo_stream";

    boost::asio::io_context ioc;
    tcp::resolver resolver{ioc};
    auto const results = resolver.resolve(host, std::to_string(port));
    tcp::socket socket{ioc};
    boost::asio::connect(socket, results.begin(), results.end());
    websocket::stream<tcp::socket> ws{std::move(socket)};
    ws.handshake(host + ":" + std::to_string(port), "/ws");

    log::info("Ingest connected to ws://" + host + ":" + std::to_string(port) + "/ws");
    long long frame = 0;
    for (;;) {
        auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            steady_clock_t::now().time_since_epoch()
        ).count();

        auto payload = mini_json::obj({
            {"stream_id", mini_json::str(stream_id)},
            {"frame_id",  mini_json::num(frame++)},
            {"ts_ms",     mini_json::num((long long)now)},
            {"bytes",     mini_json::num(0LL)}   // ✅ cast to avoid ambiguity
        });

        ws.write(boost::asio::buffer(payload));

        beast::flat_buffer buffer;
        ws.read(buffer);
        std::string reply = beast::buffers_to_string(buffer.data());
        log::info("Gateway replied: " + reply);

        std::this_thread::sleep_for(std::chrono::milliseconds(40)); // ~25fps
    }
}
