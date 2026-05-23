#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

IMAGE_NAME="${IMAGE_NAME:-unitree_lidar_imu_init:latest}"
DOCKERFILE="${DOCKERFILE:-${SCRIPT_DIR}/Dockerfile}"
BUILD_CONTEXT="${BUILD_CONTEXT:-${REPO_ROOT}}"
BUILD_NETWORK="${BUILD_NETWORK:-host}"

BUILD_ARGS=()
for proxy_name in http_proxy https_proxy all_proxy HTTP_PROXY HTTPS_PROXY ALL_PROXY; do
  if [[ -n "${!proxy_name:-}" ]]; then
    BUILD_ARGS+=(--build-arg "${proxy_name}=${!proxy_name}")
  fi
done

exec docker build \
  --network="${BUILD_NETWORK}" \
  -f "${DOCKERFILE}" \
  -t "${IMAGE_NAME}" \
  "${BUILD_ARGS[@]}" \
  "${BUILD_CONTEXT}"
