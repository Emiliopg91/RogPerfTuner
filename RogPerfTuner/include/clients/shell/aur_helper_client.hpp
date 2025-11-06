#ifdef AUR_HELPER
#pragma once

#include "../../models/others/singleton.hpp"
#include "../../utils/translator/translator.hpp"
#include "abstract/abstract_cmd_client.hpp"

class AurHelperClient : public AbstractCmdClient, public Singleton<AurHelperClient> {
  private:
	friend class Singleton<AurHelperClient>;

	AurHelperClient();

	Translator& translator = Translator::getInstance();

  public:
	std::string getVersion(std::string package);
	void install(std::string package);
};
#endif