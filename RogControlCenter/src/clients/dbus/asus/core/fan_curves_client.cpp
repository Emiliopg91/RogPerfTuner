
#include "../../../../../include/clients/dbus/asus/core/fan_curves_client.hpp"

void FanCurvesClient::resetProfileCurve(PlatformProfile profile) {
	call("ResetProfileCurves", {static_cast<uint>(profile.toInt())});
}

void FanCurvesClient::setCurveToDefaults(PlatformProfile profile) {
	call("SetCurvesToDefaults", {static_cast<uint>(profile.toInt())});
}

void FanCurvesClient::setFanCurveEnabled(PlatformProfile profile) {
	call("SetFanCurvesEnabled", {static_cast<uint>(profile.toInt()), true});
}

FanCurvesClient::FanCurvesClient() : AsusBaseClient("FanCurves") {
}