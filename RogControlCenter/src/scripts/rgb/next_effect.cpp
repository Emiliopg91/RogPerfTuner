#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "../../../include/clients/unix_socket/rog_control_center_client.hpp"
#include "../../../include/logger/logger_provider.hpp"

int main() {
	LoggerProvider::initialize();
	RogControlCenterClient::getInstance().nextEffect();
	return 0;
}