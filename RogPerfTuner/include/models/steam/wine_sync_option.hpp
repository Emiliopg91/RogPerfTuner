#pragma once

#include <array>

#include "../base/str_enum.hpp"

struct WineSyncOptionMeta {
	enum class Enum { AUTO, NTSYNC, FSYNC, ESYNC, NONE } e;
	const char* name;
	const char* val;
};

class WineSyncOption : public StrEnum<WineSyncOption, WineSyncOptionMeta::Enum, 5> {
  public:
	using Enum = WineSyncOptionMeta::Enum;
	using Base = StrEnum<WineSyncOption, Enum, 5>;
	using Base::Base;

  private:
	static constexpr std::array<WineSyncOptionMeta, 5> table{{{Enum::AUTO, "AUTO", "auto"},
															  {Enum::NTSYNC, "NTSYNC", "ntsync"},
															  {Enum::FSYNC, "FSYNC", "fsync"},
															  {Enum::ESYNC, "ESYNC", "esync"},
															  {Enum::NONE, "NONE", "none"}}};

	friend Base;
	static constexpr const std::array<WineSyncOptionMeta, 5>& metaTable() {
		return table;
	}
};
