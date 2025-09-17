#!/usr/bin/env bash
set -euo pipefail
NS=vae
kubectl apply -f k8s/namespace.yaml
kubectl apply -f k8s/inference-deploy.yaml
kubectl apply -f k8s/dispatcher-deploy.yaml
kubectl apply -f k8s/gateway-deploy.yaml
kubectl apply -f k8s/ingest-deploy.yaml
kubectl apply -f k8s/ingress.yaml
kubectl apply -f k8s/hpa-gateway.yaml || true
