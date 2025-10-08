#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "../../../include/clients/unix_socket/rog_perf_tuner_client.hpp"
#include "../../../include/logger/logger_provider.hpp"

int main() {
	LoggerProvider::initialize();
	RogPerfTunerClient::getInstance().nextProfile();
	return 0;
}