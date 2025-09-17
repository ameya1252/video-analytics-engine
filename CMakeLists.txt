cmake_minimum_required(VERSION 3.18)
project(video_analytics_engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(ENABLE_TENSORRT "Enable TensorRT/CUDA in inference service" OFF)

add_subdirectory(src/common)
add_subdirectory(src/gateway)
add_subdirectory(src/dispatcher)
add_subdirectory(src/inference)
add_subdirectory(src/ingest)
