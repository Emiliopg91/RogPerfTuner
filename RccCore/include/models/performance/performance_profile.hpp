#pragma once

#include "../base/int_enum.hpp"

struct PerformanceProfileMeta
{
    enum class Enum : int
    {
        PERFORMANCE = 2,
        BALANCED = 1,
        QUIET = 0
    } e;
    const char *name;
    int val;
};

class PerformanceProfile : public IntEnum<PerformanceProfile, PerformanceProfileMeta::Enum, PerformanceProfileMeta, 3>
{

public:
    using Enum = PerformanceProfileMeta::Enum;
    using Base = IntEnum<PerformanceProfile, Enum, PerformanceProfileMeta, 3>;
    using Base::Base;

private:
    static constexpr std::array<PerformanceProfileMeta, 3> table{{{Enum::QUIET, "QUIET", 0},
                                                                  {Enum::BALANCED, "BALANCED", 1},
                                                                  {Enum::PERFORMANCE, "PERFORMANCE", 2}}};

    static constexpr const std::array<PerformanceProfileMeta, 3> &metaTable() { return table; }

    friend Base;
};
