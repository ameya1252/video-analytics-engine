FROM ubuntu:22.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y     build-essential cmake git libboost-all-dev curl ca-certificates &&     rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . /app
RUN mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build . -j
