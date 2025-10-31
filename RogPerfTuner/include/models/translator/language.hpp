#pragma once

#include <array>

#include "../base/str_enum.hpp"

struct LanguageMeta {
	enum class Enum { DE, EN, ES } e;
	const char* name;
	const char* val;
};

class Language : public StrEnum<Language, LanguageMeta::Enum, 3> {
  public:
	using Enum = LanguageMeta::Enum;
	using Base = StrEnum<Language, Enum, 3>;
	using Base::Base;

  private:
	static constexpr std::array<LanguageMeta, 3> table{{{Enum::DE, "DE", "de"}, {Enum::EN, "EN", "en"}, {Enum::ES, "ES", "es"}}};

	friend Base;
	static constexpr const std::array<LanguageMeta, 3>& metaTable() {
		return table;
	}
};
