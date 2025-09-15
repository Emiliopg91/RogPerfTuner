#include "../../../../include/clients/dbus/asus/asus_base_client.hpp"

AsusBaseClient::AsusBaseClient(std::string interface_sufix, std::string object_path_sufix, bool required)
	: AbstractDbusClient(true, QString("xyz.ljones.Asusd"),
						 QString::fromStdString("/xyz/ljones" + (object_path_sufix.empty() ? "" : "/" + object_path_sufix)),
						 QString::fromStdString("xyz.ljones." + interface_sufix), required) {
}