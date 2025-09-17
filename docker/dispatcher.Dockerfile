FROM ubuntu:22.04 AS build
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential cmake libboost-all-dev
WORKDIR /app
COPY . /app
RUN rm -rf build && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. \
 && cmake --build . -j

FROM ubuntu:22.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y libboost-all-dev && rm -rf /var/lib/apt/lists/*
COPY --from=build /app/build/src/dispatcher/dispatcher /bin/dispatcher
ENV DISPATCHER_PORT=8090 INFERENCE_BACKENDS="inference:8091"
EXPOSE 8090
CMD ["/bin/dispatcher"]
