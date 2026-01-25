#pragma once
#include <optional>
#include <string_view>
#include <type_traits>

#include "../models/base/magin_enum.hpp"

template <typename E>
struct EnumUtils {
	static constexpr auto values() {
		return magic_enum::enum_values<E>();
	}

	static constexpr std::string_view toString(E e) {
		return magic_enum::enum_name(e);
	}

	static constexpr std::optional<E> fromString(std::string_view s) {
		return magic_enum::enum_cast<E>(s);
	}

	static constexpr int toInt(E e) {
		return static_cast<std::underlying_type_t<E>>(e);
	}

	static constexpr std::optional<E> fromInt(int v) {
		return magic_enum::enum_cast<E>(v);
	}
};
