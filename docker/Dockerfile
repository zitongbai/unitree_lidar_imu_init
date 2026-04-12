
FROM osrf/ros:noetic-desktop-full

SHELL ["/bin/bash", "-lc"]

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    iproute2 \
    iputils-ping \
    libatlas-base-dev \
    libboost-all-dev \
    libeigen3-dev \
    libapr1-dev \
    libfmt-dev \
    libglew-dev \
    libgoogle-glog-dev \
    libpcl-dev \
    libspdlog-dev \
    libsuitesparse-dev \
    libyaml-cpp-dev \
    net-tools \
    pkg-config \
    python3-pip \
    python3-tk \
    ros-noetic-eigen-conversions \
    ros-noetic-message-generation \
    ros-noetic-message-runtime \
    ros-noetic-pcl-ros \
    ros-noetic-rosbag \
    ros-noetic-roscpp \
    ros-noetic-rviz \
    ros-noetic-rospy \
    ros-noetic-sensor-msgs \
    ros-noetic-std-msgs \
    tcpdump \
    vim \
    wget \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /root

COPY docker/config/MID360_config.json /root/MID360_config.json

RUN pip3 install --no-cache-dir matplotlib

RUN wget https://github.com/ceres-solver/ceres-solver/archive/refs/tags/2.0.0.tar.gz \
 && tar zxf 2.0.0.tar.gz \
 && rm -f 2.0.0.tar.gz \
 && cd ceres-solver-2.0.0 \
 && mkdir build \
 && cd build \
 && cmake .. -DCMAKE_BUILD_TYPE=Release \
 && make -j"$(nproc)" \
 && make install

RUN git clone https://github.com/Livox-SDK/Livox-SDK2.git \
 && cd Livox-SDK2 \
 && mkdir build \
 && cd build \
 && cmake .. \
 && make -j"$(nproc)" \
 && make install

RUN git clone https://github.com/unitreerobotics/unitree_sdk2.git \
 && cd unitree_sdk2 \
 && mkdir build \
 && cd build \
 && cmake .. \
 && make -j"$(nproc)" \
 && make install

RUN mkdir -p /root/ws_livox_legacy/src \
 && git clone --branch v2.6.0 --depth 1 https://github.com/Livox-SDK/livox_ros_driver.git /root/ws_livox_legacy/src/livox_ros_driver \
 && source /opt/ros/noetic/setup.bash \
 && cd /root/ws_livox_legacy/src \
 && catkin_init_workspace \
 && cd /root/ws_livox_legacy \
 && catkin_make

RUN mkdir -p /root/catkin_ws/src \
 && git clone https://github.com/hku-mars/LiDAR_IMU_Init.git /root/catkin_ws/src/LiDAR_IMU_Init

COPY unitree_sdk2_ros_bridge /root/catkin_ws/src/unitree_sdk2_ros_bridge

RUN source /opt/ros/noetic/setup.bash \
 && source /root/ws_livox_legacy/devel/setup.bash \
 && cd /root/catkin_ws \
 && catkin_make

RUN mkdir -p ws_livox/src \
 && git clone https://github.com/Livox-SDK/livox_ros_driver2.git ws_livox/src/livox_ros_driver2 \
 && cp /root/MID360_config.json /root/ws_livox/src/livox_ros_driver2/config/MID360_config.json \
 && source /opt/ros/noetic/setup.bash \
 && cd /root/ws_livox/src/livox_ros_driver2 \
 && ./build.sh ROS1

RUN echo "source /opt/ros/noetic/setup.bash" >> /root/.bashrc \
 && echo "source /root/ws_livox_legacy/devel/setup.bash" >> /root/.bashrc \
 && echo "source /root/catkin_ws/devel/setup.bash" >> /root/.bashrc \
 && echo "source /root/ws_livox/devel/setup.bash" >> /root/.bashrc
