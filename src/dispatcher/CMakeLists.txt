find_package(Boost REQUIRED COMPONENTS system thread)
add_executable(dispatcher main.cpp)
target_link_libraries(dispatcher PRIVATE Boost::system Boost::thread common)
