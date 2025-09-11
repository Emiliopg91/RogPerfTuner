#pragma once
#include <string>
#include <string_view>

template <typename T>
struct UsbIdentifierT {
	T id_vendor;
	T id_product;
	T name;

	// Comparación genérica con otro UsbIdentifierT<U>
	template <typename U>
	constexpr bool operator==(const UsbIdentifierT<U>& other) const noexcept {
		return id_vendor == other.id_vendor && id_product == other.id_product;
	}
};

// Alias convenientes
using UsbIdentifier		= UsbIdentifierT<std::string>;		 // runtime
using UsbIdentifierView = UsbIdentifierT<std::string_view>;	 // compile-time