#include <array>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include <ros/ros.h>
#include <sensor_msgs/Imu.h>

#include <unitree/idl/hg/IMUState_.hpp>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>

namespace
{
constexpr char kDefaultSecondaryImuTopic[] = "rt/secondary_imu";

std::array<double, 3> LoadCovarianceDiagonal(
    const ros::NodeHandle& nh,
    const std::string& param_name,
    const std::array<double, 3>& default_value)
{
  std::vector<double> values;
  if (!nh.getParam(param_name, values))
  {
    return default_value;
  }

  if (values.size() != 3U)
  {
    throw std::runtime_error("Parameter '" + param_name + "' must contain exactly 3 elements.");
  }

  return {values[0], values[1], values[2]};
}

void SetCovarianceDiagonal(boost::array<double, 9>& covariance, const std::array<double, 3>& diagonal)
{
  covariance.assign(0.0);
  covariance[0] = diagonal[0];
  covariance[4] = diagonal[1];
  covariance[8] = diagonal[2];
}
}  // namespace

class G1ImuBridge
{
public:
  G1ImuBridge(ros::NodeHandle nh, ros::NodeHandle pnh)
      : nh_(std::move(nh)), pnh_(std::move(pnh))
  {
    pnh_.param<std::string>("network_interface", network_interface_, "");
    pnh_.param<std::string>("imu_topic", imu_topic_, "/g1/secondary_imu/data");
    pnh_.param<std::string>("frame_id", frame_id_, "g1_secondary_imu_link");
    pnh_.param<std::string>("unitree_imu_topic", unitree_imu_topic_, kDefaultSecondaryImuTopic);

    orientation_covariance_diagonal_ = LoadCovarianceDiagonal(
        pnh_, "orientation_covariance_diagonal", {1e-3, 1e-3, 1e-3});
    angular_velocity_covariance_diagonal_ = LoadCovarianceDiagonal(
        pnh_, "angular_velocity_covariance_diagonal", {1e-3, 1e-3, 1e-3});
    linear_acceleration_covariance_diagonal_ = LoadCovarianceDiagonal(
        pnh_, "linear_acceleration_covariance_diagonal", {1e-2, 1e-2, 1e-2});

    if (network_interface_.empty())
    {
      throw std::runtime_error("Parameter '~network_interface' is required.");
    }

    imu_publisher_ = nh_.advertise<sensor_msgs::Imu>(imu_topic_, 10);

    unitree::robot::ChannelFactory::Instance()->Init(0, network_interface_);

    imu_subscriber_.reset(
        new unitree::robot::ChannelSubscriber<unitree_hg::msg::dds_::IMUState_>(unitree_imu_topic_));
    imu_subscriber_->InitChannel(
        std::bind(&G1ImuBridge::HandleImuState, this, std::placeholders::_1), 1);
  }

private:
  void HandleImuState(const void* message)
  {
    const auto& imu_state = *static_cast<const unitree_hg::msg::dds_::IMUState_*>(message);

    sensor_msgs::Imu imu_msg;
    imu_msg.header.stamp = ros::Time::now();
    imu_msg.header.frame_id = frame_id_;

    const auto& quaternion = imu_state.quaternion();
    imu_msg.orientation.w = quaternion[0];
    imu_msg.orientation.x = quaternion[1];
    imu_msg.orientation.y = quaternion[2];
    imu_msg.orientation.z = quaternion[3];

    const auto& gyroscope = imu_state.gyroscope();
    imu_msg.angular_velocity.x = gyroscope[0];
    imu_msg.angular_velocity.y = gyroscope[1];
    imu_msg.angular_velocity.z = gyroscope[2];

    const auto& accelerometer = imu_state.accelerometer();
    imu_msg.linear_acceleration.x = accelerometer[0];
    imu_msg.linear_acceleration.y = accelerometer[1];
    imu_msg.linear_acceleration.z = accelerometer[2];

    SetCovarianceDiagonal(imu_msg.orientation_covariance, orientation_covariance_diagonal_);
    SetCovarianceDiagonal(imu_msg.angular_velocity_covariance, angular_velocity_covariance_diagonal_);
    SetCovarianceDiagonal(imu_msg.linear_acceleration_covariance, linear_acceleration_covariance_diagonal_);

    imu_publisher_.publish(imu_msg);
  }

  ros::NodeHandle nh_;
  ros::NodeHandle pnh_;
  ros::Publisher imu_publisher_;

  std::string network_interface_;
  std::string imu_topic_;
  std::string frame_id_;
  std::string unitree_imu_topic_;
  std::array<double, 3> orientation_covariance_diagonal_{};
  std::array<double, 3> angular_velocity_covariance_diagonal_{};
  std::array<double, 3> linear_acceleration_covariance_diagonal_{};

  unitree::robot::ChannelSubscriberPtr<unitree_hg::msg::dds_::IMUState_> imu_subscriber_;
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "g1_imu_bridge");
  ros::NodeHandle nh;
  ros::NodeHandle pnh("~");

  try
  {
    G1ImuBridge bridge(nh, pnh);
    const std::string unitree_imu_topic =
        pnh.param<std::string>("unitree_imu_topic", std::string(kDefaultSecondaryImuTopic));
    const std::string imu_topic =
        pnh.param<std::string>("imu_topic", std::string("/g1/secondary_imu/data"));
    ROS_INFO("g1_imu_bridge is listening on Unitree topic '%s' and publishing ROS topic '%s'.",
             unitree_imu_topic.c_str(),
             imu_topic.c_str());
    ros::waitForShutdown();
  }
  catch (const std::exception& ex)
  {
    ROS_FATAL("%s", ex.what());
    return 1;
  }

  return 0;
}
