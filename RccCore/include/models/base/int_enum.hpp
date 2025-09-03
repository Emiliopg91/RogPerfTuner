#pragma once

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// --------------------
// Base genérica
// --------------------
template <typename Derived, typename EnumType, typename MetaType, size_t N>
class IntEnum {
  public:
	using Enum = EnumType;

	IntEnum(Enum e) : value(e) {
	}
	IntEnum(int val) : value(fromInt(val)) {
	}

	int toInt() const {
		return static_cast<int>(value);
	}

	std::string toName() const {
		for (size_t i = 0; i < N; ++i)
			if (Derived::metaTable()[i].val == static_cast<int>(value))
				return Derived::metaTable()[i].name;
		return "UNKNOWN";
	}

	bool operator==(const IntEnum& other) const {
		return value == other.value;
	}
	bool operator!=(const IntEnum& other) const {
		return value != other.value;
	}

	friend std::ostream& operator<<(std::ostream& os, const IntEnum& obj) {
		os << obj.toName() << "(" << obj.toInt() << ")";
		return os;
	}

	static Enum fromInt(int v) {
		for (size_t i = 0; i < N; ++i)
			if (Derived::metaTable()[i].val == v)
				return Derived::metaTable()[i].e;
		throw std::invalid_argument("Unknown value: " + std::to_string(v));
	}

	static std::vector<Enum> getAll() {
		std::vector<Enum> all;

		const auto& table = Derived::metaTable();
		for (size_t i = 0; i < N; ++i)
			all.push_back(table[i].e);

		return all;
	}

  protected:
	Enum value;
};