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
| `diffbot_controller` | Config/Launch | ros2_control controller config and spawner launch (joint_state_broadcaster, velocity controller) |

## TF flow

```
/joint_states (wheel angles)
        +
/robot_description (URDF geometry)
        ↓  [robot_state_publisher computes the kinematics]
/tf and /tf_static (3D pose of each link)
```

## ros2_control

Wheel velocity control goes through `ros2_control` instead of talking to the Gazebo plugin directly:

- **Hardware interface** (`diffbot_description/urdf/diffbot_ros2_control.xacro`): declares `wheel_left_joint`/`wheel_right_joint` with a `velocity` command interface and `position`/`velocity` state interfaces, backed by the `ign_ros2_control/IgnitionSystem` plugin (Humble) or `gz_ros2_control/GazeboSimSystem` (Iron+) — picked automatically via `is_ignition`, set from `ROS_DISTRO` in `gazebo.launch.py`.
- **Gazebo plugin** (`diffbot_description/urdf/diffbot_gazebo.xacro`): loads the matching `ign_ros2_control-system`/`gz_ros2_control-system` plugin, which starts the `controller_manager` using `diffbot_controller/config/diffbot_controllers.yaml`.
- **Controllers** (`diffbot_controller/config/diffbot_controllers.yaml`): `joint_state_broadcaster` (publishes `/joint_states`) and `simple_velocity_controller` (`velocity_controllers/JointGroupVelocityController`, takes commands for `[wheel_left_joint, wheel_right_joint]` in that order).
- **Spawner launch** (`diffbot_controller/launch/controller.launch.py`): spawns both controllers via `controller_manager/spawner` — run it in a separate terminal after Gazebo is up.

```sh
# terminal 1
ros2 launch diffbot_description gazebo.launch.py

# terminal 2, once Gazebo has loaded
ros2 launch diffbot_controller controller.launch.py

# terminal 3, drive the wheels directly (bypassing any future diff-drive/odometry layer)
ros2 topic pub -r 10 /simple_velocity_controller/commands std_msgs/msg/Float64MultiArray "{data: [0.5, 0.5]}"
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

- **Apt packages installed**:

  ```sh
  sudo apt update
  sudo apt install -y ros-humble-ros-gz                                      # Gazebo Fortress + ros_gz_sim/ros_gz_bridge
  sudo apt install -y ros-humble-ign-ros2-control ros-humble-ros2-controllers  # ros2_control + controller_manager
  ```
