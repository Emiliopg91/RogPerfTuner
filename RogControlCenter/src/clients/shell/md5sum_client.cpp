#include "../../../include/clients/shell/md5sum_client.hpp"

#include "../../../include/utils/string_utils.hpp"

std::string Md5SumClient::getChecksum(const std::string& file) {
	return StringUtils::split(run_command(file, true).stdout_str, ' ')[0];
}