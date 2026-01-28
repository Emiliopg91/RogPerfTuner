#include "clients/file/cpuinfo_client.hpp"

CPUInfoClient::CPUInfoClient() : AbstractFileClient("/proc/cpuinfo", "CPUInfoClient") {
}