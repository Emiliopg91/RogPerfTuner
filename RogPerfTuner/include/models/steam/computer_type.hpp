#pragma once

#include <array>

#include "../base/str_enum.hpp"

struct ComputerTypeMeta {
	enum class Enum { COMPUTER, STEAM_DECK, STEAM_MACHINE } e;
	const char* name;
	const char* val;
};

class ComputerType : public StrEnum<ComputerType, ComputerTypeMeta::Enum, 3> {
  public:
	using Enum = ComputerTypeMeta::Enum;
	using Base = StrEnum<ComputerType, Enum, 3>;
	using Base::Base;

	int getPresetIndex();

  private:
	static constexpr std::array<ComputerTypeMeta, 3> table{{{Enum::COMPUTER, "COMPUTER", "computer"},
															{Enum::STEAM_DECK, "STEAM_DECK", "steam_deck"},
															{Enum::STEAM_MACHINE, "STEAM_MACHINE", "steam_machine"}}};

	static constexpr const std::array<ComputerTypeMeta, 3>& metaTable() {
		return table;
	}

	friend Base;
};
