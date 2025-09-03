#pragma once

#include "../base/str_enum.hpp"

struct SsdSchedulerMeta {
	enum class Enum { NOOP, MQ_DEADLINE } e;
	const char* name;
	const char* val;
};

class SsdScheduler : public StrEnum<SsdScheduler, SsdSchedulerMeta::Enum, 2> {
  public:
	using Enum = SsdSchedulerMeta::Enum;
	using Base = StrEnum<SsdScheduler, Enum, 2>;
	using Base::Base;

  private:
	static constexpr std::array<SsdSchedulerMeta, 2> table{
		{{Enum::NOOP, "NOOP", "none"}, {Enum::MQ_DEADLINE, "MQ_DEADLINE", "mq-deadline"}}};

	friend Base;
	static constexpr const std::array<SsdSchedulerMeta, 2>& metaTable() {
		return table;
	}
};
