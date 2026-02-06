#include "clients/file/intel_rapl_uj_client.hpp"

IntelRaplUJClient::IntelRaplUJClient() : AbstractFileClient("/sys/class/powercap/intel-rapl:0/energy_uj", "IntelRaplUJClient", false, false) {
}

void IntelRaplUJClient::enableRead() {
	shell.run_elevated_command("chmod o+r " + this->path_, false);
}