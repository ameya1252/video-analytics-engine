#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "../common/net.hpp"
#include "../common/json.hpp"
#include "../common/consistent_hash.hpp"
#include "../common/logging.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
using tcp = boost::asio::ip::tcp;

struct Session : public std::enable_shared_from_this<Session> {
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    ConsistentHash& ring_;
    explicit Session(tcp::socket socket, ConsistentHash& ring) : ws_(std::move(socket)), ring_(ring) {}
    void run() {
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res) {
                res.set(http::field::server, "Gateway/1.0");
            }));
        // Accept the websocket handshake
        ws_.async_accept(beast::bind_front_handler(&Session::on_accept, shared_from_this()));
    }
    void on_accept(beast::error_code ec) {
        if (ec) return log::error("ws accept: " + ec.message());
        do_read();
    }
    void do_read() {
        ws_.async_read(buffer_, beast::bind_front_handler(&Session::on_read, shared_from_this()));
    }
    void on_read(beast::error_code ec, std::size_t bytes) {
        if (ec == websocket::error::closed) return;
        if (ec) { log::error("ws read: " + ec.message()); return; }
        std::string msg = beast::buffers_to_string(buffer_.data());
        buffer_.consume(buffer_.size());

        // crude parse of stream_id (looking for "stream_id":"...")
        std::string stream_id = "unknown";
        auto key = std::string("\"stream_id\":\"");
        auto p = msg.find(key);
        if (p != std::string::npos) {
            auto s = p + key.size();
            auto e = msg.find('"', s);
            if (e != std::string::npos) stream_id = msg.substr(s, e - s);
        }
        auto backend = ring_.route(stream_id);
        auto out = mini_json::obj({
            {"ok", mini_json::boolean(true)},
            {"bytes", mini_json::num((long long)bytes)},
            {"backend", mini_json::str(backend)},
        });
        ws_.text(true);
        ws_.async_write(boost::asio::buffer(out), [self = shared_from_this()](beast::error_code ec, std::size_t){
            if (ec) log::error("ws write: " + ec.message());
        });
        do_read();
    }
};

struct Listener : public std::enable_shared_from_this<Listener> {
    boost::asio::io_context& ioc_;
    tcp::acceptor acceptor_;
    ConsistentHash& ring_;
    Listener(boost::asio::io_context& ioc, tcp::endpoint ep, ConsistentHash& ring) 
        : ioc_(ioc), acceptor_(ioc), ring_(ring) {
        beast::error_code ec;
        acceptor_.open(ep.protocol(), ec);
        if (ec) { log::error("open: " + ec.message()); return; }
        acceptor_.set_option(boost::asio::socket_base::reuse_address(true));
        acceptor_.bind(ep, ec);
        if (ec) { log::error("bind: " + ec.message()); return; }
        acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
        if (ec) { log::error("listen: " + ec.message()); return; }
    }
    void run() { do_accept(); }
    void do_accept() {
        acceptor_.async_accept(boost::asio::make_strand(ioc_), beast::bind_front_handler(&Listener::on_accept, shared_from_this()));
    }
    void on_accept(beast::error_code ec, tcp::socket socket) {
        if (ec) { log::error("accept: " + ec.message()); }
        else std::make_shared<Session>(std::move(socket), ring_)->run();
        do_accept();
    }
};

int main(int argc, char** argv) {
    int port = 8080;
    if (const char* p = std::getenv("GATEWAY_PORT")) port = std::atoi(p);
    std::vector<std::string> backends;
    if (const char* b = std::getenv("INFERENCE_BACKENDS")) {
        // comma-separated host:port, e.g., "inference:8091,inference2:8091"
        std::string s(b);
        size_t start = 0;
        while (true) {
            auto pos = s.find(',', start);
            backends.push_back(s.substr(start, pos - start));
            if (pos == std::string::npos) break;
            start = pos + 1;
        }
    } else {
        backends = {"localhost:8091"};
    }

    ConsistentHash ring(100);
    ring.set_backends(backends);

    boost::asio::io_context ioc;
    Listener server(ioc, tcp::endpoint(tcp::v4(), port), ring);
    server.run();
    log::info("Gateway listening on ws://0.0.0.0:" + std::to_string(port) + "/ws");
    ioc.run();
    return 0;
}
