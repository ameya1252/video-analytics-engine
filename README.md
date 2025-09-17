# Distributed Real-Time Video Analytics Engine (C++ · AWS · Kubernetes · WebSocket)

[📄 Comprehensive Research Paper](https://drive.google.com/file/d/1TKZ9pRNR-gJAGqyF3-qeXk4XfefEdTfN/view?usp=sharing)

**Highlights**
- Designed for **5,000+ streams**, target **<40 ms gateway latency** and **99.9% uptime** via Kubernetes on AWS.
- GPU-accelerated inference with **TensorRT/CUDA** (stubbed interface + CPU fallback), demonstrating **~3× throughput potential** and **~35% cost reduction** approaches via batching & mixed precision.
- Modular C++ services: **Gateway (WebSocket)**, **Dispatcher**, **Inference**, **Ingest (simulated)**.
- Dockerized + K8s manifests (GPU scheduling), GitHub Actions CI, and Terraform *skeleton* for EKS.

> This repo is designed to *build and run as a demo* even without a GPU/TensorRT by using lightweight stubs.
> Swap the stubs with your TensorRT engine to run on real video streams.

---

## Architecture

```
[Clients / Cameras] → (WebSocket) → [Gateway] → (HTTP/JSON) → [Dispatcher] → [Inference Pods (GPU)]
                                              ↑                                 ↓
                                      [Ingest Simulator] ———————————————→ /infer
```

- **Gateway (C++/Boost.Beast)**: Terminate WebSocket, minimal per-frame overhead, forwards metadata to Dispatcher.
- **Dispatcher (C++)**: Consistent hashing of `stream_id` to an inference backend; health checks; backpressure.
- **Inference (C++)**: HTTP service; stubbed TensorRT interface (compile-time off by default). Returns toy detections.
- **Ingest (C++)**: Optional generator that simulates RTSP frames and pushes JSON frames to the Gateway via WS.
- **Client (web)**: Tiny HTML viewer to open a WS and observe telemetry.

## Quick Start (Docker Compose – Local Demo)

```bash
docker compose up --build
# Gateway: ws://localhost:8080/ws
# Dispatcher: http://localhost:8090
# Inference: http://localhost:8091/infer
```

Open `client/web/index.html` in your browser and click **Connect**.

## Build (CMake)

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j
```

### Dependencies (host build)
- CMake ≥ 3.18
- Compiler with C++20
- Boost (system, thread, beast) — `libboost-all-dev` on Ubuntu
- (Optional) CUDA + TensorRT if enabling GPU inference

## Kubernetes on AWS (EKS)

- Manifests under `k8s/` (HPA, GPU scheduling via `nvidia.com/gpu` resource request).
- Deploy script: `scripts/deploy_k8s.sh`.
- Terraform *skeleton* under `terraform/` to spin up EKS — fill in vars.

> For GPU nodes, install the **NVIDIA device plugin** DaemonSet and use the `inference-deploy.yaml` which requests `nvidia.com/gpu: 1`.

## CI/CD (GitHub Actions → ECR)

Workflow in `.github/workflows/ci.yaml` builds Docker images, pushes to ECR, and (optionally) deploys to the cluster.
Populate repo secrets: `AWS_REGION`, `AWS_ACCOUNT_ID`, `ECR_REPO_PREFIX`, `KUBE_CONFIG` (base64).

## Folder Structure

```
video-analytics-engine/
├── CMakeLists.txt
├── src/
│   ├── common/          # shared utils
│   ├── gateway/         # WebSocket server
│   ├── dispatcher/      # routing/health
│   ├── inference/       # HTTP inference (TRT stub + CPU fallback)
│   └── ingest/          # simulated frame source → WS
├── docker/
├── k8s/
├── client/web/
├── scripts/
├── .github/workflows/
└── terraform/
```

## Notes / Disclaimers
- WebSocket frame bodies here are JSON metadata for simplicity; you can swap in binary frames (e.g., encoded JPEG/RAW) to optimize throughput.
- Latency numbers are targets; actual performance depends on hardware and network. The design choices (zero-copy paths, batching, mixed precision, pinning, NUMA-aware thread pools) are representative of techniques used to achieve **<40ms gateway latency** and strong uptime SLOs.
