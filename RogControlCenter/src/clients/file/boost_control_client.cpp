#include "../../../include/clients/file/boost_control_client.hpp"

void BoostControlClient::set_boost(bool& enabled) {
	write(enabled ? on : off);
}

BoostControlClient::BoostControlClient(const std::string& path, const std::string& on, const std::string& off)
	: AbstractFileClient(path, "BoostControlClient", true), on(on), off(off) {
}