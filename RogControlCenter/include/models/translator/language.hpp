#pragma once

#include <array>

#include "../base/str_enum.hpp"

struct LanguageMeta {
	enum class Enum { EN, ES } e;
	const char* name;
	const char* val;
};

class Language : public StrEnum<Language, LanguageMeta::Enum, 2> {
  public:
	using Enum = LanguageMeta::Enum;
	using Base = StrEnum<Language, Enum, 2>;
	using Base::Base;

  private:
	static constexpr std::array<LanguageMeta, 2> table{{{Enum::ES, "ES", "es"}, {Enum::EN, "EN", "en"}}};

	friend Base;
	static constexpr const std::array<LanguageMeta, 2>& metaTable() {
		return table;
	}
};
