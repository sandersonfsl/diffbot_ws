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

## TF flow

```
/joint_states (wheel angles)
        +
/robot_description (URDF geometry)
        ↓  [robot_state_publisher computes the kinematics]
/tf and /tf_static (3D pose of each link)
```

## Build

```sh
cd diffbot_ws
colcon build
. install/setup.bash
```

## Notes

- **Gazebo inside Docker**: if `ros2 launch diffbot_description gazebo.launch.py` starts but the spawn node (`ros_gz_sim create`) loops forever on `Requesting list of world names` and the Gazebo GUI stays black/frozen, Ignition Transport is likely picking the wrong network interface for discovery (common on hosts with multiple NICs/Docker bridges). Fix by pinning discovery to loopback before launching:

  ```sh
  export GZ_IP=127.0.0.1
  export IGN_IP=127.0.0.1
  ros2 launch diffbot_description gazebo.launch.py
  ```
