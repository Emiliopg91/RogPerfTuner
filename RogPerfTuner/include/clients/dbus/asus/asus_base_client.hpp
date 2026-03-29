#pragma once

#include "framework/clients/abstract/abstract_dbus_client.hpp"

class AsusBaseClient : public AbstractDbusClient {
  public:
	AsusBaseClient(const std::string& interface_sufix, const std::string& object_path_sufix = "", bool required = false);
};