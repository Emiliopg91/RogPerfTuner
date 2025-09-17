#include "../../../include/models/steam/mangohud_level.hpp"

int MangoHudLevel::getPresetIndex() {
	switch (this->value) {
		case MangoHudLevelMeta::Enum::NO_DISPLAY:
			return 0;
		case MangoHudLevelMeta::Enum::FPS_ONLY:
			return 1;
		case MangoHudLevelMeta::Enum::HORIZONTAL_VIEW:
			return 2;
		case MangoHudLevelMeta::Enum::EXTENDED:
			return 3;
		case MangoHudLevelMeta::Enum::HIGH_DETAILED:
			return 4;
	}
}