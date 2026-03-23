#include "clients/file/intel_rapl_uj_client.hpp"

IntelRaplUJClient::IntelRaplUJClient() : AbstractFileClient(INTEL_RAPL_UJ_FILE, "IntelRaplUJClient") {
}

void IntelRaplUJClient::enableRead() {
	shell.run_elevated_command("chmod o+r " + this->path_, false);
}