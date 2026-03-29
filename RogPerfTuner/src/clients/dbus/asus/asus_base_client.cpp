#include "clients/dbus/asus/asus_base_client.hpp"

AsusBaseClient::AsusBaseClient(const std::string& interface_sufix, const std::string& object_path_sufix, bool required)
	: AbstractDbusClient(true, QString("xyz.ljones.Asusd"), ("/xyz/ljones" + (object_path_sufix.empty() ? "" : "/" + object_path_sufix)).c_str(),
						 ("xyz.ljones." + interface_sufix).c_str(), required) {
}