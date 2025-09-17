find_package(Boost REQUIRED COMPONENTS system thread)
add_executable(gateway main.cpp)
target_link_libraries(gateway PRIVATE Boost::system Boost::thread common)
