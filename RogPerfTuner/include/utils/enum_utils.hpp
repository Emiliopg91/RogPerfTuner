#pragma once

#include <algorithm>
#include <any>
#include <optional>
#include <string>
#include <type_traits>

#include "../models/base/magin_enum.hpp"
#include "string_utils.hpp"

template <typename T>
concept EnumClassInt = std::is_enum_v<T> && std::is_same_v<std::underlying_type_t<T>, int>;

template <EnumClassInt E>
static auto values(bool reversed = false) {
	auto v = magic_enum::enum_values<E>();

	if (reversed) {
		std::reverse(v.begin(), v.end());
	}

	return v;
}

template <EnumClassInt E>
static std::string toName(E e) {
	return std::string(magic_enum::enum_name<E>(e));
}

template <EnumClassInt E>
static E fromName(std::string s) {
	std::optional<E> v = magic_enum::enum_cast<E>(s);

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid name " + s;
}

template <EnumClassInt E>
static std::string toString(E e, const std::unordered_map<std::string, std::string>& replacement = {}) {
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

template <EnumClassInt E>
static E fromString(std::string s, const std::unordered_map<std::string, std::string>& replacement = {}) {
	auto v = std::string(s);

	for (const auto& [key, value] : replacement) {
		size_t pos = 0;
		while ((pos = v.find(key, pos)) != std::string::npos) {
			v.replace(pos, key.length(), value);
			pos += value.length();
		}
	}

	return fromName<E>(StringUtils::toUpperCase(s));
}

template <EnumClassInt E>
static int toInt(E e) {
	return static_cast<std::underlying_type_t<E>>(e);
}

template <EnumClassInt E>
static E fromInt(int i) {
	std::optional<E> v = magic_enum::enum_cast<E>(i);

	if (v.has_value()) {
		return *v;
	}

	throw "Invalid value " + std::to_string(i);
}