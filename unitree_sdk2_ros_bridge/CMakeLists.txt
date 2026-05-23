cmake_minimum_required(VERSION 3.0.2)
project(unitree_sdk2_ros_bridge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  sensor_msgs
)

find_package(Threads REQUIRED)
find_package(unitree_sdk2 CONFIG REQUIRED)

catkin_package()

include_directories(
  SYSTEM
  ${catkin_INCLUDE_DIRS}
)

add_executable(go2_imu_bridge
  src/go2_imu_bridge.cpp
)

add_executable(g1_imu_bridge
  src/g1_imu_bridge.cpp
)

target_link_libraries(go2_imu_bridge
  ${catkin_LIBRARIES}
  unitree_sdk2
  Threads::Threads
)

target_link_libraries(g1_imu_bridge
  ${catkin_LIBRARIES}
  unitree_sdk2
  Threads::Threads
)

install(TARGETS go2_imu_bridge g1_imu_bridge
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(FILES
  launch/go2_imu_bridge.launch
  launch/g1_imu_bridge.launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/launch
)
