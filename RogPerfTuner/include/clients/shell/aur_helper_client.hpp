#ifdef AUR_HELPER
#pragma once
#include "framework/abstracts/singleton.hpp"
#include "framework/clients/abstract/abstract_cmd_client.hpp"
#include "framework/translator/translator.hpp"

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