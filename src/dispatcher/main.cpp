#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

#include "../common/consistent_hash.hpp"
#include "../common/json.hpp"
#include "../common/logging.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

class HttpServer {
    boost::asio::io_context ioc_;
    tcp::acceptor acceptor_;
    ConsistentHash ring_;
public:
    HttpServer(int port, const std::vector<std::string>& backends)
        : acceptor_(ioc_, tcp::endpoint(tcp::v4(), port)) {
        ring_.set_backends(backends);
    }

    void run() {
        for (;;) {
            tcp::socket socket{ioc_};
            acceptor_.accept(socket);
            std::thread(&HttpServer::handle, this, std::move(socket)).detach();
        }
    }

    void handle(tcp::socket socket) {
        beast::error_code ec;
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);
        if (ec) return;

        if (req.method() == http::verb::get && req.target().starts_with("/health")) {
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.body() = "{\"ok\":true}";
            res.prepare_payload();
            http::write(socket, res);
            return;
        }

        if (req.method() == http::verb::post && req.target().starts_with("/assign")) {
            // crude parse of stream_id from body
            std::string body = req.body();
            std::string stream_id = "unknown";
            auto key = std::string("\"stream_id\":\"");
            auto p = body.find(key);
            if (p != std::string::npos) {
                auto s = p + key.size();
                auto e = body.find('\"', s);
                if (e != std::string::npos) stream_id = body.substr(s, e - s);
            }

            auto backend = ring_.route(stream_id);
            auto out = mini_json::obj({
                {"backend", mini_json::str(backend)},
                {"ok", mini_json::boolean(true)}
            });
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.body() = out;
            res.prepare_payload();
            http::write(socket, res);
            return;
        }

        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::content_type, "text/plain");
        res.body() = "not found";
        res.prepare_payload();
        http::write(socket, res);
    }
};

int main() {
    int port = 8090;
    if (const char* p = std::getenv("DISPATCHER_PORT")) port = std::atoi(p);

    std::vector<std::string> backends;
    if (const char* b = std::getenv("INFERENCE_BACKENDS")) {
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

    log::info("Dispatcher on http://0.0.0.0:" + std::to_string(port));
    HttpServer srv(port, backends);
    srv.run();
    return 0;
}
