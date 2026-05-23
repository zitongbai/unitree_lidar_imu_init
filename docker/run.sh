#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MID360_CONFIG="${SCRIPT_DIR}/config/mid360.yaml"
CONTAINER_MID360_CONFIG="/root/catkin_ws/src/LiDAR_IMU_Init/config/mid360.yaml"

IMAGE_NAME="${IMAGE_NAME:-unitree_lidar_imu_init:latest}"
CONTAINER_NAME="${CONTAINER_NAME:-unitree_lidar_imu_init}"

if [[ ! -f "${MID360_CONFIG}" ]]; then
  echo "Missing config file: ${MID360_CONFIG}" >&2
  exit 1
fi

if [[ -n "${DISPLAY:-}" ]]; then
  xhost +local:root >/dev/null
  DISPLAY_ARGS=(
    --env "DISPLAY=${DISPLAY}"
    --env "QT_X11_NO_MITSHM=1"
    --volume "/tmp/.X11-unix:/tmp/.X11-unix:rw"
  )
else
  DISPLAY_ARGS=()
fi

exec docker run --rm -it \
  --name "${CONTAINER_NAME}" \
  --net=host \
  "${DISPLAY_ARGS[@]}" \
  --volume "${MID360_CONFIG}:${CONTAINER_MID360_CONFIG}:ro" \
  "${IMAGE_NAME}" bash
