cmake_minimum_required(VERSION 3.8)
project(robo_gazebo_sim)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(urdf REQUIRED)
find_package(xacro REQUIRED)
find_package(ros_gz_sim REQUIRED)
find_package(ros_gz_bridge REQUIRED)
find_package(ros_gz_interfaces REQUIRED)
# [NOVO] Acha a API do MoveIt para compilar o executável em C++
find_package(moveit_ros_planning_interface REQUIRED)

# [NOVO] Configura a compilação do executável CLI pick_place_cli
add_executable(pick_place_cli src/pick_place_cli.cpp)
target_include_directories(pick_place_cli PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(pick_place_cli PUBLIC c_std_99 cxx_std_17)  # Require C++17
ament_target_dependencies(
  pick_place_cli
  "rclcpp"
  "moveit_ros_planning_interface"
  "geometry_msgs"
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# Install non-code resources used at runtime (launch files, URDF/Xacro, params).
install(
  DIRECTORY launch model parameters
  DESTINATION share/${PROJECT_NAME}
)

# [NOVO] Instala o executável para poder ser rodado via ros2 run
install(TARGETS pick_place_cli
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}/
)

ament_package()
