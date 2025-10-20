#include "../../../include/clients/shell/md5_client.hpp"

#include "../../../include/utils/string_utils.hpp"

Md5SumClient::Md5SumClient() : AbstractCmdClient("md5sum", "Md5SumClient") {
}

const std::string Md5SumClient::checksum(const std::string& path) {
	auto out	  = StringUtils::trim(run_command(path).stdout_str);
	auto outParts = StringUtils::split(out, ' ');

	return outParts[0];
}