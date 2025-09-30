#include <exception>
#include <iostream>

#include "../../../include/clients/unix_socket/rog_control_center_client.hpp"
#include "../../../include/logger/logger_provider.hpp"

int main() {
	LoggerProvider::initialize();

	try {
		std::cout << RogControlCenterClient::getInstance().nextEffect() << std::endl;
		return 0;
	} catch (std::exception& e) {
		std::cerr << "Error on request: " << e.what() << std::endl;
	}
}