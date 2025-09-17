# --- build stage ---
    FROM ubuntu:22.04 AS build
    RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential cmake libboost-all-dev
    WORKDIR /app
    COPY . /app
    # 👇 clean build dir, then configure + compile
    RUN rm -rf build && mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. \
     && cmake --build . -j
    
    # --- runtime stage ---
    FROM ubuntu:22.04
    RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y libboost-all-dev && rm -rf /var/lib/apt/lists/*
    COPY --from=build /app/build/src/gateway/gateway /bin/gateway
    ENV GATEWAY_PORT=8080 INFERENCE_BACKENDS="inference:8091"
    EXPOSE 8080
    CMD ["/bin/gateway"]
    