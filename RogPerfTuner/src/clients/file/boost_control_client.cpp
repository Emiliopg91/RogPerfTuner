#include "clients/file/boost_control_client.hpp"

#include "framework/clients/abstract/abstract_file_client.hpp"

BoostControlClient::BoostControlClient() : AbstractFileClient(BOOST_CONTROL_FILE, "BoostControlClient", true) {
}

void BoostControlClient::set_boost(bool& enabled) {
	write(enabled ? BOOST_CONTROL_ON : BOOST_CONTROL_OFF);
}