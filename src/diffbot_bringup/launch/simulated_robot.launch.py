import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    world_name_arg = DeclareLaunchArgument(name="world_name", default_value="empty")

    use_slam = LaunchConfiguration("use_slam")
    use_slam_arg = DeclareLaunchArgument(name="use_slam", default_value="false")

    gazebo = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("diffbot_description"),
            "launch",
            "gazebo.launch.py"
        ),
        launch_arguments={
            "world_name": LaunchConfiguration("world_name")
        }.items(),
    )

    controller = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("diffbot_controller"),
            "launch",
            "controller.launch.py"
        ),
        launch_arguments={
            "use_simple_controller": "False"
        }.items(),
    )

    joystick = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("diffbot_controller"),
            "launch",
            "joystick_teleop.launch.py"
        ),
        launch_arguments={
            "use_sim_time": "True"
        }.items()
    )

    # Local sensor fusion (EKF: wheel odom + IMU -> base_footprint_ekf). Always
    # on, independent of the global map-relative choice below (AMCL vs SLAM).
    local_localization = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("diffbot_localization"),
            "launch",
            "local_localization.launch.py"
        ),
    )

    safety_stop = Node(
        package="diffbot_utils",
        executable="safety_stop",
        output="screen",
        parameters=[{"use_sim_time": True}]
    )

    # Global, map-relative localization: AMCL against a known map (default) or
    # live SLAM to build one, mutually exclusive via use_slam.
    global_localization = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("diffbot_localization"),
            "launch",
            "global_localization.launch.py"
        ),
        condition=UnlessCondition(use_slam)
    )

    slam = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("diffbot_mapping"),
            "launch",
            "slam.launch.py"
        ),
        condition=IfCondition(use_slam)
    )

    rviz_localization = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", os.path.join(
                get_package_share_directory("diffbot_localization"),
                "rviz",
                "global_localization.rviz"
            )
        ],
        output="screen",
        parameters=[{"use_sim_time": True}],
        condition=UnlessCondition(use_slam)
    )

    rviz_slam = Node(
        package="rviz2",
        executable="rviz2",
        arguments=["-d", os.path.join(
                get_package_share_directory("diffbot_mapping"),
                "rviz",
                "slam.rviz"
            )
        ],
        output="screen",
        parameters=[{"use_sim_time": True}],
        condition=IfCondition(use_slam)
    )

    return LaunchDescription([
        world_name_arg,
        use_slam_arg,
        gazebo,
        controller,
        joystick,
        safety_stop,
        local_localization,
        global_localization,
        slam,
        rviz_localization,
        rviz_slam,
    ])
