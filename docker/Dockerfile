FROM ros:humble-ros-base-jammy

ENV DEBIAN_FRONTEND=noninteractive

# Base dev tools only. ROS package dependencies (ros2_control, diff_drive_controller,
# nav2_amcl, slam_toolbox, twist_mux, robot_localization, joy, libserial-dev,
# i2c-tools, python3-smbus, ...) are NOT hardcoded here — they're declared once in
# each package's package.xml and pulled in via `rosdep install` against the mounted
# diffbot_ws/src (see docs/raspberry_pi_setup.md), so this image never drifts out of
# sync with what the workspace actually needs.
RUN apt-get update && apt-get install -y \
    tmux \
    vim \
    git \
    python3-pip \
    python3-colcon-common-extensions \
    python3-rosdep \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install smbus2

RUN rosdep update

WORKDIR /root

RUN echo "source /opt/ros/humble/setup.bash" >> /root/.bashrc

CMD ["/bin/bash"]
