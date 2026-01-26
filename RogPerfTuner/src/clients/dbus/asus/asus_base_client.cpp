#include "../../../../include/clients/dbus/asus/asus_base_client.hpp"

AsusBaseClient::AsusBaseClient(std::string interface_sufix, std::string object_path_sufix, bool required)
	: AbstractDbusClient(true, QString("xyz.ljones.Asusd"), ("/xyz/ljones" + (object_path_sufix.empty() ? "" : "/" + object_path_sufix)).c_str(),
						 ("xyz.ljones." + interface_sufix).c_str(), required) {
}