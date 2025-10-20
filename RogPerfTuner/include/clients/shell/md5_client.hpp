#pragma once

#include "../../models/others/singleton.hpp"
#include "abstract/abstract_cmd_client.hpp"

class Md5SumClient : public AbstractCmdClient, public Singleton<Md5SumClient> {
  private:
	friend class Singleton<Md5SumClient>;

	Md5SumClient();

  public:
	/**
	 * @brief Calculates the MD5 checksum of the file at the specified path.
	 *
	 * This function computes and returns the MD5 hash of the contents of the file
	 * located at the given path. The result is returned as a hexadecimal string.
	 *
	 * @param path The path to the file whose checksum is to be calculated.
	 * @return std::string The MD5 checksum as a hexadecimal string.
	 */
	const std::string checksum(const std::string& path);
};