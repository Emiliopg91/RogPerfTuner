#pragma once
#include <string>

struct UsbIdentifier {
	std::string id_vendor;
	std::string id_product;
	std::string name;

	constexpr bool operator==(const UsbIdentifier& other) const noexcept {
		return id_vendor == other.id_vendor && id_product == other.id_product;
	}
};