# unitree_lidar_imu_init

This repository provides a **Docker-only** workflow to identify the **extrinsic calibration** (relative position and orientation) between a Unitree robot's onboard IMU and a LiDAR sensor (primarily **Livox MID-360**).

It is designed for the Unitree family of robots (e.g. Go2) where you want to estimate the rigid transform between:

- **LiDAR frame** (Livox MID-360)
- **IMU frame** (Unitree onboard IMU exposed via Unitree SDK2 `LowState`)

To avoid ROS/Ubuntu version mismatches and simplify setup, this repository **only supports Docker deployment**.

## Acknowledgements / Upstream Dependency

This project is built on top of **@hku-mars/LiDAR_IMU_Init** and uses it as the core initialization / estimation pipeline:

- `hku-mars/LiDAR_IMU_Init`

This repository does **not** vendor the upstream code; the Docker image clones and builds it during the image build stage.

## Why Unitree SDK2 Instead of Unitree ROS2

The core calibration pipeline comes from [`hku-mars/LiDAR_IMU_Init`](https://github.com/hku-mars/LiDAR_IMU_Init.git), which is a ROS1 package. Because the calibration node runs in ROS1 / ROS Noetic, this repository cannot directly use Unitree's ROS2 interface as the IMU source.

Instead, the Docker image builds `unitree_sdk2` and this repository provides a small ROS1 bridge package. The bridge reads Unitree SDK2 `LowState` IMU data and republishes it as ROS1 `sensor_msgs/Imu`, allowing it to be consumed by `LiDAR_IMU_Init`.

## What This Repository Provides

- A Docker environment based on `osrf/ros:noetic-desktop-full`
- A ROS1 bridge package that converts Unitree SDK2 `LowState` IMU data into `sensor_msgs/Imu`
- An integrated Docker build that fetches and builds required upstream dependencies to run `LiDAR_IMU_Init`

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
- access to the network interface connected to the Unitree robot

## Build Docker Image

Use host networking during build so the Docker build steps can access a host-side proxy such as `127.0.0.1:10808`.

```bash
docker build --network=host -f docker/Dockerfile -t unitree_lidar_imu_init:latest .
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

### 2. Start Unitree IMU bridge

```bash
docker exec -it <container_name_or_id> bash
cd /root/catkin_ws
source devel/setup.bash
roslaunch unitree_sdk2_ros_bridge go2_imu_bridge.launch network_interface:=enp8s0
```

`network_interface` must be the host/container NIC that can reach the robot.

By default, the bridge publishes IMU data to:

```text
/mavros/imu/data_raw
```

That topic name is chosen so it can be consumed by `LiDAR_IMU_Init` without extra remapping.

### 3. Run LiDAR-IMU initialization (extrinsic calibration)

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

- The setup is optimized for Docker-based execution rather than native host installation.
- The exact network interface name depends on the machine. Replace `enp8s0` with your actual NIC.
- Successful runtime depends on correct Livox and Unitree network configuration outside this repository.
- Upstream dependencies keep their own licenses.

## License

The ROS bridge package is released under the MIT license.
