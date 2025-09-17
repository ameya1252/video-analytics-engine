find_package(Boost REQUIRED COMPONENTS system thread)
add_executable(ingest main.cpp)
target_link_libraries(ingest PRIVATE Boost::system Boost::thread common)
