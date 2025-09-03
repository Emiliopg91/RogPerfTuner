#pragma once

#include "../abstract/abstract_dbus_client.hpp"

class AsusBaseClient : public AbstractDbusClient {
  public:
	AsusBaseClient(std::string interface_sufix, std::string object_path_sufix = "", bool required = false)
		: AbstractDbusClient(
			  true, QString("xyz.ljones.Asusd"),
			  QString::fromStdString("/xyz/ljones" + (object_path_sufix.empty() ? "" : "/" + object_path_sufix)),
			  QString::fromStdString("xyz.ljones." + interface_sufix), required) {
	}
};