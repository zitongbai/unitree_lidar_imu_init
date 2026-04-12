# Docker

当前 `docker/Dockerfile` 已经包含并验证通过以下安装：
- `Livox-SDK2`
- `livox_ros_driver2`
- `unitree_sdk2`
- `docker/config/MID360_config.json` 会在构建时覆盖容器内的 `livox_ros_driver2/config/MID360_config.json`

## Build

```bash
docker build -f docker/Dockerfile -t go2_lidar_imu_init:test .
```

## Run

推荐使用宿主机网络启动。这样容器内可以直接接收通过网线打到宿主机网卡上的 Livox 雷达 UDP 数据。

```bash
xhost +local:root

docker run --rm -it \
  --net=host \
  --env DISPLAY=$DISPLAY \
  --env QT_X11_NO_MITSHM=1 \
  --volume /tmp/.X11-unix:/tmp/.X11-unix:rw \
  --device /dev/dri \
  go2_lidar_imu_init:test bash
```

## In Container

如果容器已经在运行，先查看容器名或容器 ID：

```bash
docker ps
```

然后进入该容器：

```bash
docker exec -it <container_name_or_id> bash
```

例如：

```bash
docker exec -it hopeful_morse bash
```

容器启动后会自动 `source`：

```bash
source /opt/ros/noetic/setup.bash
source /root/ws_livox/devel/setup.bash
```

常用检查命令：

```bash
ip addr
ip route
ping 192.168.123.200
tcpdump -i <your_nic> udp
```

## Run

### 终端1

进入容器：
```bash
docker exec -it <container_name_or_id> bash
```

获取并发布mid360点云：

```bash
cd /root/ws_livox
source devel/setup.bash
roslaunch livox_ros_driver2 msg_MID360.launch
```

### 终端2

进入容器：
```bash
docker exec -it <container_name_or_id> bash
```

获取并发布IMU数据：

```bash
cd /root/catkin_ws
source devel/setup.bash
roslaunch unitree_sdk2_ros_bridge go2_imu_bridge.launch network_interface:=enp8s0
```

### 终端3

进入容器：
```bash
docker exec -it <container_name_or_id> bash
```

运行 LiDAR-IMU 初始化：

```bash
cd /root/catkin_ws
source devel/setup.bash
roslaunch lidar_imu_init livox_mid360.launch
```
