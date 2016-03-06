#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>
#include <geometry_msgs/Twist.h>
#include "high_level_control.h"

HighLevelControl::HighLevelControl() : node_() {
    security_distance_ = 0.25;
    wall_follow_distance_ = 0.35;
    linear_velocity_ = 0.5;
    angular_velocity_ = 0.314;

    cmd_vel_pub_ = node_.advertise<geometry_msgs::Twist>("cmd_vel", 100);
    laser_sub_ = node_.subscribe("base_scan", 100, &HighLevelControl::LaserCallback, this);
    can_continue_ = false;
    is_close_to_wall_ = false;
    is_turning_ = false;
}

void HighLevelControl::LaserCallback(const sensor_msgs::LaserScan::ConstPtr& msg) {
    std::vector<float> ranges(msg->ranges.begin(), msg->ranges.end());
    std::vector<float>::iterator it = std::min_element(ranges.begin(), ranges.end());
    std::vector<float>::iterator right_it = std::min_element(ranges.begin() + 50, ranges.begin() + 150);

    if (*it > security_distance_) {
        can_continue_ = true;
    } else {
        can_continue_ = false;
    }

    if (*right_it < wall_follow_distance_) {
        is_close_to_wall_ = true;
    } else {
        is_close_to_wall_ = false;
    }
}

void HighLevelControl::set_security_distance(double security_distance) {
    security_distance_ = security_distance;
}

void HighLevelControl::set_linear_velocity(double linear_velocity) {
    linear_velocity_ = linear_velocity;
}

void HighLevelControl::set_angular_velocity(double angular_velocity) {
    angular_velocity_ = angular_velocity;
}

void HighLevelControl::WallFollowMove() {
    if (can_continue_ && is_close_to_wall_) {
        Move(linear_velocity_, 0);
    } else if (!can_continue_) {
        Move(0, 2 * angular_velocity_);
    } else if (!is_close_to_wall_) {
        Move(0, -2 * angular_velocity_);
    }
}

void HighLevelControl::RandomMove() {
    if (can_continue_) {
        Move(linear_velocity_, 0);
        is_turning_ = false;
    } else {
        if (!is_turning_) {
            turn_type_ = rand() % 2 == 0 ? -3 : 3;
            is_turning_ = true;
        }

        Move(0, angular_velocity_ * turn_type_);
    }
}

//TODO: To be put into Low Level Control
void HighLevelControl::Move(double linear_velocity, double angular_velocity) {
    geometry_msgs::Twist msg;
    msg.linear.x = linear_velocity;
    msg.angular.z = angular_velocity;
    cmd_vel_pub_.publish(msg);
}