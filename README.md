# DiffBot

An autonomous differential-drive robot built from scratch in ROS 2 — control, odometry, localization, SLAM, path planning, and navigation.

Following the base structure of Antonio Brandi's "Self-Driving and ROS 2 - Learn by Doing" reimplemented

## Roadmap

- **Phase 1 — Control & Odometry**: ROS2 basics, locomotion, control, kinematics, TF2, odometry, sensor fusion (EKF). 
- **Phase 2 — Localization & Mapping**: probability, global localization, sensors, maps, SLAM. 
- **Phase 3 — Planning & Navigation**: path/motion planning, obstacle avoidance, Nav2, decision making. 

## Structure

```
diffbot_ws/         # real ROS2 workspace (colcon) — code lives and builds here
```

## Packages

| Package | Language | Purpose |
| --- | --- | --- |
| `diffbot_cpp_examples` | C++ | Core ROS2 concepts (publisher, subscriber, service, action...) |
| `diffbot_description` | URDF/Xacro | 3D robot model, visual/collision geometry, joint definitions, and simulation parameters |
| `diffbot_msgs` | Interfaces | Custom msg/srv/action definitions used across DiffBot packages |

## Build

```sh
cd diffbot_ws
colcon build
. install/setup.bash
```
