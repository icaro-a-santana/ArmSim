import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node

def generate_launch_description():
    pkg_so100_desc = get_package_share_directory('so100_description')
    pkg_ros_gz_sim = get_package_share_directory('ros_gz_sim')

    urdf_file = os.path.join(pkg_so100_desc, 'share', 'so100_description', 'so100.urdf')
    if not os.path.exists(urdf_file):
        urdf_file = os.path.join(pkg_so100_desc, 'so100.urdf')

    with open(urdf_file, 'r') as infp:
        robot_desc = infp.read()

    # Define os caminhos de busca para as malhas 3D no Gazebo Harmonic
    set_gz_resource_path = SetEnvironmentVariable(
        name='GZ_SIM_RESOURCE_PATH',
        value=[
            os.path.join(pkg_so100_desc, '..'),
            ':' + os.environ.get('GZ_SIM_RESOURCE_PATH', '')
        ]
    )

    # Inicializa a cena no Gazebo Sim
    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_ros_gz_sim, 'launch', 'gz_sim.launch.py')
        ),
        launch_arguments={'gz_args': '-r empty.sdf'}.items(),
    )

    # Publica a árvore de transformação (TF) do robô
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='both',
        parameters=[{'robot_description': robot_desc, 'use_sim_time': True}]
    )

    # Instancia a entidade do robô dentro do simulador ativo
    spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-string', robot_desc,
            '-name', 'so_arm_100',
            '-z', '0.05'
        ],
        output='screen'
    )

    return LaunchDescription([
        set_gz_resource_path,
        gz_sim,
        robot_state_publisher,
        spawn_entity
    ])
