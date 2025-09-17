#pragma once
#include <boost/asio.hpp>
#include <string>

namespace net {
    using tcp = boost::asio::ip::tcp;
    using error_code = boost::system::error_code;
}
