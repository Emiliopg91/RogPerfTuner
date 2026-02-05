#pragma once

#include <magic_enum.hpp>
#include <ranges>
#include <string>
#include <type_traits>
#include <unordered_map>

#include "framework/utils/string_utils.hpp"

template <typename T>
concept EnumClass = std::is_enum_v<T>;

template <typename T>
concept EnumClassInt = EnumClass<T> && std::is_same_v<std::underlying_type_t<T>, int>;

template <EnumClass E, bool Reversed = false>
constexpr auto values() {
	if constexpr (Reversed) {
		return magic_enum::enum_values<E>() | std::views::reverse;
	} else {
		return magic_enum::enum_values<E>();
	}
}

template <EnumClass E>
constexpr std::string toName(const E& e) {
	return std::string(magic_enum::enum_name<E>(e));
}

template <EnumClass E>
E fromName(const std::string& s) {
	if (auto v = magic_enum::enum_cast<E>(s)) {
		return *v;
	}
	throw std::runtime_error("Invalid name '" + s + "'");
}

template <EnumClass E>
constexpr std::string toString(const E& e, const std::unordered_map<std::string, std::string>& replacement = {}) {
	auto v = StringUtils::toLowerCase(toName(e));

	for (const auto& [key, value] : replacement) {
		size_t pos = 0;
		while ((pos = v.find(key, pos)) != std::string::npos) {
			v.replace(pos, key.length(), value);
			pos += value.length();
		}
	}

	return v;
}

template <EnumClass E>
E fromString(const std::string& s, const std::unordered_map<std::string, std::string>& replacement = {}) {
	std::string v = s;
	for (const auto& [key, value] : replacement) {
		size_t pos = 0;
		while ((pos = v.find(key, pos)) != std::string::npos) {
			v.replace(pos, key.length(), value);
			pos += value.length();
		}
	}

	return fromName<E>(StringUtils::toUpperCase(v));
}

template <EnumClassInt E>
constexpr int toInt(const E& e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

template <EnumClassInt E>
E fromInt(const int& i) {
	if (auto v = magic_enum::enum_cast<E>(i)) {
		return *v;
	}
	throw std::runtime_error("Invalid value '" + std::to_string(i) + "'");
}
