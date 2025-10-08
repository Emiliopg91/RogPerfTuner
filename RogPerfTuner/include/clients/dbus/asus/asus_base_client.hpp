#pragma once

#include "../abstract/abstract_dbus_client.hpp"

class AsusBaseClient : public AbstractDbusClient {
  public:
	AsusBaseClient(std::string interface_sufix, std::string object_path_sufix = "", bool required = false);
};