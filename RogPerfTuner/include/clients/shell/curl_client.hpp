#pragma once

#include <unordered_map>

#include "clients/shell/abstract/abstract_cmd_client.hpp"
#include "models/others/singleton.hpp"

class CurlClient : public AbstractCmdClient, public Singleton<CurlClient> {
  private:
	friend class Singleton<CurlClient>;

	std::optional<std::string> current;
	std::unordered_map<std::string, std::string> available_sched;

	CurlClient();

  public:
	void download(std::string url, std::string dst);

	std::string fetch(std::string url);
};