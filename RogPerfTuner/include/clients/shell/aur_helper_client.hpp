#ifdef AUR_HELPER
#pragma once
#include "abstracts/clients/abstract_cmd_client.hpp"
#include "abstracts/singleton.hpp"
#include "translator/translator.hpp"

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