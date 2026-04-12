# unitree_lidar_imu_init

ROS1 tooling for running LiDAR-IMU initialization with a Unitree Go2 IMU and a Livox MID-360 LiDAR.

This repository provides:

- a Docker environment based on `osrf/ros:noetic-desktop-full`
- a ROS1 bridge package that converts Unitree SDK2 `LowState` IMU data into `sensor_msgs/Imu`
- an integrated setup that builds the upstream dependencies needed to run [`LiDAR_IMU_Init`](https://github.com/hku-mars/LiDAR_IMU_Init)

The main goal is to make the Go2 IMU usable in a standard ROS1 LiDAR-IMU initialization pipeline without manually stitching together Unitree SDK2, Livox drivers, and the initialization package.

## Repository Layout

```text
.
|-- docker/
|   |-- Dockerfile
|   `-- config/MID360_config.json
`-- unitree_sdk2_ros_bridge/
    |-- launch/go2_imu_bridge.launch
    |-- src/go2_imu_bridge.cpp
    |-- CMakeLists.txt
    `-- package.xml
```

## What Gets Built in Docker

The Docker image installs and builds:

- ROS Noetic desktop environment
- `Livox-SDK2`
- `livox_ros_driver2`
- legacy `livox_ros_driver` v2.6.0
- `unitree_sdk2`
- upstream `LiDAR_IMU_Init`
- this repository's `unitree_sdk2_ros_bridge`

During image build, `docker/config/MID360_config.json` replaces the default `MID360_config.json` inside `livox_ros_driver2`.

## Prerequisites

- Docker
- an X11 environment if you want to use GUI tools such as RViz
- a host network setup that can receive Livox MID-360 UDP traffic
- access to the network interface connected to the Unitree Go2

This project is currently set up around:

- Ubuntu 20.04 style ROS1 workflow
- ROS Noetic
- Livox MID-360
- Unitree Go2 low state IMU stream over Unitree SDK2

## Build Docker Image

```bash
docker build -f docker/Dockerfile -t unitree_lidar_imu_init:latest .
```

## Run Docker Container

Host networking is recommended so the container can directly receive Livox UDP packets.

```bash
xhost +local:root

docker run --rm -it \
  --net=host \
  --env DISPLAY=$DISPLAY \
  --env QT_X11_NO_MITSHM=1 \
  --volume /tmp/.X11-unix:/tmp/.X11-unix:rw \
  --device /dev/dri \
  unitree_lidar_imu_init:latest bash
```

Once inside the container, the following workspaces are sourced automatically from `/root/.bashrc`:

- `/opt/ros/noetic/setup.bash`
- `/root/ws_livox_legacy/devel/setup.bash`
- `/root/catkin_ws/devel/setup.bash`
- `/root/ws_livox/devel/setup.bash`

## Runtime Workflow

Open three terminals in the running container.

### 1. Start Livox MID-360 driver

```bash
docker exec -it <container_name_or_id> bash
cd /root/ws_livox
source devel/setup.bash
roslaunch livox_ros_driver2 msg_MID360.launch
```

### 2. Start Go2 IMU bridge

```bash
docker exec -it <container_name_or_id> bash
cd /root/catkin_ws
source devel/setup.bash
roslaunch unitree_sdk2_ros_bridge go2_imu_bridge.launch network_interface:=enp8s0
```

`network_interface` must be the host/container NIC that can reach the Go2.

By default, the bridge publishes IMU data to:

```text
/mavros/imu/data_raw
```

That topic name is intentionally chosen so it can be consumed by `LiDAR_IMU_Init` without extra remapping.

### 3. Run LiDAR-IMU initialization

```bash
docker exec -it <container_name_or_id> bash
cd /root/catkin_ws
source devel/setup.bash
roslaunch lidar_imu_init livox_mid360.launch
```

## Bridge Package

The package in [`unitree_sdk2_ros_bridge`](./unitree_sdk2_ros_bridge) subscribes to the Unitree SDK2 low-state DDS topic and republishes the IMU fields as ROS `sensor_msgs/Imu`.

Default bridge behavior:

- subscribes to Unitree topic `rt/lowstate`
- publishes ROS topic `/mavros/imu/data_raw`
- sets `frame_id` to `go2_imu_link`
- fills covariance diagonals from ROS params

### Launch Parameters

The launch file [`go2_imu_bridge.launch`](./unitree_sdk2_ros_bridge/launch/go2_imu_bridge.launch) exposes:

- `network_interface`: NIC used by Unitree SDK2. Required for actual communication.
- `imu_topic`: ROS IMU output topic. Default: `/mavros/imu/data_raw`
- `frame_id`: IMU frame id. Default: `go2_imu_link`
- `unitree_low_state_topic`: DDS topic name. Default: `rt/lowstate`

Covariance parameters:

- `orientation_covariance_diagonal`: default `[0.001, 0.001, 0.001]`
- `angular_velocity_covariance_diagonal`: default `[0.001, 0.001, 0.001]`
- `linear_acceleration_covariance_diagonal`: default `[0.01, 0.01, 0.01]`

Example:

```bash
roslaunch unitree_sdk2_ros_bridge go2_imu_bridge.launch \
  network_interface:=enp8s0 \
  imu_topic:=/mavros/imu/data_raw \
  frame_id:=go2_imu_link
```

## Useful Diagnostics

Inside the container, these commands are usually the first things to check:

```bash
ip addr
ip route
ping 192.168.123.200
tcpdump -i <your_nic> udp
rostopic list
rostopic echo /mavros/imu/data_raw
```

## Notes and Limitations

- This repository does not vendor `LiDAR_IMU_Init`; the Docker build clones it from the upstream repository.
- The setup is currently optimized for Docker-based execution rather than native host installation.
- The exact network interface name depends on the machine. Replace `enp8s0` with your actual NIC.
- Successful runtime depends on correct Livox and Unitree network configuration outside this repository.

## License

The ROS bridge package is released under the MIT license. Upstream dependencies keep their own licenses.
