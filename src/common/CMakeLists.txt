add_library(common STATIC
    logging.cpp
    net.hpp
    json.hpp
    consistent_hash.hpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
find_package(Boost REQUIRED COMPONENTS system)
target_link_libraries(common PUBLIC Boost::system)
