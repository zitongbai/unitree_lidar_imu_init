#!/usr/bin/env bash
set -euo pipefail

CONTAINER_NAME="${CONTAINER_NAME:-unitree_lidar_imu_init}"

if ! docker ps --format '{{.Names}}' | grep -Fxq "${CONTAINER_NAME}"; then
  echo "Container is not running: ${CONTAINER_NAME}" >&2
  echo "Start it first with docker/run.sh, or set CONTAINER_NAME=<name>." >&2
  exit 1
fi

exec docker exec -it "${CONTAINER_NAME}" bash
