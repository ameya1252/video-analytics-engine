FROM ubuntu:22.04 AS build
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential cmake libboost-all-dev
WORKDIR /app
COPY . /app
RUN rm -rf build && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. \
 && cmake --build . -j

FROM ubuntu:22.04
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y libboost-all-dev && rm -rf /var/lib/apt/lists/*
COPY --from=build /app/build/src/ingest/ingest /bin/ingest
ENV GATEWAY_HOST=gateway GATEWAY_PORT=8080 STREAM_ID=demo_stream
CMD ["/bin/ingest"]
