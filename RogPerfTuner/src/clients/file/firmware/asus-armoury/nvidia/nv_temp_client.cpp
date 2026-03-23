#include "clients/file/firmware/asus-armoury/nvidia/nv_temp_client.hpp"

NvTempClient::NvTempClient() : ArmouryBaseClient(NVIDIA_THERMAL_FILE, "NvTempClient", false) {
}