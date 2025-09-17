find_package(Boost REQUIRED COMPONENTS system thread)
add_executable(inference main.cpp trt_infer.cpp)
target_link_libraries(inference PRIVATE Boost::system Boost::thread common)
target_compile_definitions(inference PRIVATE $<$<BOOL:${ENABLE_TENSORRT}>:USE_TRT>)
