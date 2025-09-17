#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "../common/json.hpp"
#include "../common/logging.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

struct Detection { float x,y,w,h,score; std::string label; };
std::vector<Detection> run_inference_stub(const std::string& frame);

class Server {
    boost::asio::io_context ioc_;
    tcp::acceptor acceptor_;
public:
    Server(int port) : acceptor_(ioc_, tcp::endpoint(tcp::v4(), port)) {}
    void run() {
        for (;;) {
            tcp::socket socket{ioc_};
            acceptor_.accept(socket);
            std::thread(&Server::handle, this, std::move(socket)).detach();
        }
    }
    void handle(tcp::socket socket) {
        beast::error_code ec;
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);
        if (ec) return;

        if (req.method()==http::verb::get && req.target()=="/health") {
            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::content_type, "application/json");
            res.body() = "{\"ok\":true}";
            res.prepare_payload();
            http::write(socket, res);
            return;
        }

        if (req.method()==http::verb::post && req.target().starts_with("/infer")) {
            // In a real system parse bytes; here we just stub.
            auto dets = run_inference_stub(req.body());
            std::ostringstream arr;
            arr << "[";
            for (size_t i=0;i<dets.size();++i){
                const auto& d = dets[i];
                arr << mini_json::obj({
                    {"x", mini_json::num(d.x)},
                    {"y", mini_json::num(d.y)},
                    {"w", mini_json::num(d.w)},
                    {"h", mini_json::num(d.h)},
                    {"score", mini_json::num(d.score)},
                    {"label", mini_json::str(d.label)}
                });
                if (i+1<dets.size()) arr << ",";
            }
            arr << "]";
            auto out = mini_json::obj({
                {"ok", mini_json::boolean(true)},
                {"detections", arr.str()}
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

int main(){
    int port = 8091;
    if (const char* p = std::getenv("INFERENCE_PORT")) port = std::atoi(p);
    log::info("Inference on http://0.0.0.0:" + std::to_string(port));
    Server s(port); s.run();
    return 0;
}
