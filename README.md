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
| `diffbot_controller` | Config/Launch/C++ | ros2_control controllers, the `simple_controller` differential-kinematics node, and joystick teleop |
| `diffbot_localization` | Config/Launch/C++ | Sensor fusion — `imu_republisher` re-frames `/imu/out` to `base_footprint_ekf`; `robot_localization` EKF (`ekf.yaml`) fuses it with noisy wheel odometry (`odom_noisy`) |

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

Two mutually exclusive control paths are registered in `diffbot_controllers.yaml`, picked at launch time via `use_simple_controller`:

- **`simple_controller` path** (`use_simple_controller:=True`, default) — a hand-built path for learning the kinematics:
  - `joint_state_broadcaster`: publishes `/joint_states` from the hardware interface's state_interfaces.
  - `simple_velocity_controller` (`velocity_controllers/JointGroupVelocityController`): writes to command_interfaces, takes `[wheel_left_joint, wheel_right_joint]` velocities in that order.
  - `diffbot_controller/src/simple_controller.cpp`: subscribes `geometry_msgs/msg/TwistStamped` on `/diffbot_controller/cmd_vel`, applies the differential-drive inverse kinematics (`wheel_radius`/`wheel_separation` params, default `0.033`/`0.1402203698837279` from the URDF geometry) and publishes wheel velocities to `/simple_velocity_controller/commands`. No odometry, no cmd_vel timeout — it's the manual version of what `diff_drive_controller` does below.
- **`diffbot_controller` path** (`use_simple_controller:=False`) — the same `wheel_left_joint`/`wheel_right_joint` on `/diffbot_controller/cmd_vel`, but through the built-in `diff_drive_controller/DiffDriveController`: also publishes `/odom` + TF, enforces velocity/acceleration limits, and has a `cmd_vel_timeout` (0.5s) that zeroes commands if `cmd_vel` stops arriving.

Run either with:

```sh
ros2 launch diffbot_controller controller.launch.py                            # simple_controller (default)
ros2 launch diffbot_controller controller.launch.py use_simple_controller:=False  # diff_drive_controller
```

## Joystick teleop

`diffbot_controller/launch/joystick_teleop.launch.py` publishes `TwistStamped` on `/diffbot_controller/cmd_vel` from a gamepad (tested with a Logitech F710), via `joy_node` + `joy_teleop`:

```sh
ros2 launch diffbot_controller joystick_teleop.launch.py
```

Axis mapping and deadman button are in `diffbot_controller/config/joy_teleop.yaml`; joystick device settings (deadzone, autorepeat rate) in `joy_config.yaml`. `joy_node` resolves `device_id` by SDL2 enumeration order among currently connected joysticks, not by `/dev/input/jsN` number.

## Comparing odometry (PlotJuggler)

With Gazebo, `controller.launch.py` and `local_localization.launch.py` all running, install and open PlotJuggler:

```sh
sudo apt install -y ros-humble-plotjuggler-ros
ros2 run plotjuggler plotjuggler
```

Streaming → Start: "ROS2 Topic Subscriber", pick `/diffbot_controller/odom` (truth), `/diffbot_controller/odom_noisy` (noisy), and `/odometry/filtered` (EKF output). Plot `pose/pose/position/x` vs `.../y` from each on the same XY plot to compare trajectories visually.


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

- **Dependencies**: declared per-package in each `package.xml`. Install them all with:

  ```sh
  rosdep update
  rosdep install --from-paths src --ignore-src -r -y
  ```
