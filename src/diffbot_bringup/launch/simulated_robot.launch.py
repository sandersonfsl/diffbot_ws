import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    gazebo = IncludeLaunchDescription(
        os.path.join(
            get_package_share_directory("diffbot_description"),
            "launch",
            "gazebo.launch.py"
        ),
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

    localization = IncludeLaunchDescription(
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

    return LaunchDescription([
        gazebo,
        controller,
        joystick,
        safety_stop,
        localization,
    ])
