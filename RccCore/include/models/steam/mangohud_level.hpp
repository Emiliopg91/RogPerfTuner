#pragma once

#include "../base/int_enum.hpp"
// --------------------
// Meta independiente
// --------------------
struct MangoHudLevelMeta
{
    enum class Enum : int
    {
        NO_DISPLAY = 0,
        FPS_ONLY = 1,
        HORIZONTAL_VIEW = 2,
        EXTENDED = 3,
        HIGH_DETAILED = 4
    } e;
    const char *name;
    int val;
};

class MangoHudLevel : public IntEnum<MangoHudLevel, MangoHudLevelMeta::Enum, MangoHudLevelMeta, 5>
{
public:
    using Enum = MangoHudLevelMeta::Enum;
    using Base = IntEnum<MangoHudLevel, Enum, MangoHudLevelMeta, 5>;
    using Base::Base;

private:
    static constexpr std::array<MangoHudLevelMeta, 5> table{{{Enum::NO_DISPLAY, "NO_DISPLAY", 0},
                                                             {Enum::FPS_ONLY, "FPS_ONLY", 1},
                                                             {Enum::HORIZONTAL_VIEW, "HORIZONTAL_VIEW", 2},
                                                             {Enum::EXTENDED, "EXTENDED", 3},
                                                             {Enum::HIGH_DETAILED, "HIGH_DETAILED", 4}}};

    static constexpr const std::array<MangoHudLevelMeta, 5> &metaTable() { return table; }

    friend Base;
};
