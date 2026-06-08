// [ARQUIVO NOVO] Script em C++ para ler as coordenadas do terminal e executar as trajetórias com o MoveIt
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <moveit/move_group_interface/move_group_interface.hpp>
#include <iostream>

int main(int argc, char * argv[])
{
  // Inicializa o ROS 2 e seus argumentos de linha de comando
  rclcpp::init(argc, argv);
  
  // Cria o nó principal do ROS 2 para o nosso script CLI. 
  // Configuramos 'automatically_declare_parameters_from_overrides' como true para que
  // ele aceite parâmetros vindos do launch file (ou da linha de comando) automaticamente,
  // o que é essencial para o MoveIt puxar coisas como robot_description.
  auto const node = std::make_shared<rclcpp::Node>(
    "pick_place_cli",
    rclcpp::NodeOptions().automatically_declare_parameters_from_overrides(true)
  );

  // O MoveIt requer que o nó rode em background para processar chamadas de serviço,
  // tópicos e actions do ROS. Aqui criamos um executor e rodamos ele numa thread separada.
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node);
  std::thread([&executor]() { executor.spin(); }).detach();

  // Instancia a interface de controle do MoveIt para o braço ("arm") e a garra ("gripper")
  // Esses grupos foram criados no arquivo 'ugv.srdf' do robo_moveit_config
  using moveit::planning_interface::MoveGroupInterface;
  auto move_group_interface = MoveGroupInterface(node, "arm");
  auto gripper_interface = MoveGroupInterface(node, "gripper");

  std::cout << "=== Pick and Place CLI ===" << std::endl;
  
  // Lê as coordenadas 3D de 'Pick' informadas pelo usuário
  double px, py, pz;
  std::cout << "Digite as coordenadas de Pick (X Y Z): ";
  if (!(std::cin >> px >> py >> pz)) return 1;

  // Lê as coordenadas 3D de 'Place' informadas pelo usuário
  double dx, dy, dz;
  std::cout << "Digite as coordenadas de Place (X Y Z): ";
  if (!(std::cin >> dx >> dy >> dz)) return 1;

  // --- ETAPA 1: Mover para o local de Pick ---
  std::cout << "Movendo para Pick..." << std::endl;
  geometry_msgs::msg::Pose target_pose1;
  target_pose1.orientation.w = 1.0; // Mantém a orientação neutra (quaternion)
  target_pose1.position.x = px;
  target_pose1.position.y = py;
  target_pose1.position.z = pz;
  
  // Define o alvo cartesiano para o braço e tenta planejar a trajetória
  move_group_interface.setPoseTarget(target_pose1);
  moveit::planning_interface::MoveGroupInterface::Plan my_plan;
  
  if (move_group_interface.plan(my_plan) == moveit::core::MoveItErrorCode::SUCCESS) {
    // Se o planejamento encontrou uma trajetória válida e sem colisões, executa o movimento no Gazebo/RViz
    move_group_interface.execute(my_plan);
  } else {
    std::cout << "Falha ao planejar movimento para Pick!" << std::endl;
  }

  // --- ETAPA 2: Fechar a garra para pegar o objeto ---
  std::cout << "Fechando Garra..." << std::endl;
  // Utiliza um "estado nomeado" pré-definido no SRDF ('close') para facilitar o fechamento
  gripper_interface.setNamedTarget("close");
  if (gripper_interface.plan(my_plan) == moveit::core::MoveItErrorCode::SUCCESS) {
    gripper_interface.execute(my_plan);
  }

  // --- ETAPA 3: Mover para o local de Place ---
  std::cout << "Movendo para Place..." << std::endl;
  geometry_msgs::msg::Pose target_pose2;
  target_pose2.orientation.w = 1.0;
  target_pose2.position.x = dx;
  target_pose2.position.y = dy;
  target_pose2.position.z = dz;
  
  // Atualiza o alvo com a posição de destino e repete o processo
  move_group_interface.setPoseTarget(target_pose2);
  if (move_group_interface.plan(my_plan) == moveit::core::MoveItErrorCode::SUCCESS) {
    move_group_interface.execute(my_plan);
  } else {
    std::cout << "Falha ao planejar movimento para Place!" << std::endl;
  }

  // --- ETAPA 4: Abrir a garra e soltar o objeto ---
  std::cout << "Abrindo Garra..." << std::endl;
  // Utiliza o "estado nomeado" pré-definido ('open') do SRDF
  gripper_interface.setNamedTarget("open");
  if (gripper_interface.plan(my_plan) == moveit::core::MoveItErrorCode::SUCCESS) {
    gripper_interface.execute(my_plan);
  }

  std::cout << "Tarefa concluída." << std::endl;
  
  // Encerra a execução do ROS 2 de forma limpa
  rclcpp::shutdown();
  return 0;
}
