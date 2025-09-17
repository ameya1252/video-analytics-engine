# API – Minimal Demo

## Dispatcher
- `POST /assign` body: `{ "stream_id": "string" }`
  - Response: `{ "ok": true, "backend": "host:port" }`

## Inference
- `GET /health`
- `POST /infer` body: arbitrary bytes/JSON (stubbed)
  - Response: `{ "ok": true, "detections": [ {x,y,w,h,score,label}, ... ] }`

## Gateway (WebSocket)
- Path: `/ws`
- Send frames as JSON: `{ "stream_id": "s", "frame_id": 123, "ts_ms": 0, "bytes": 0 }`
- Server replies with: `{ "ok": true, "bytes": N, "backend": "..." }`
