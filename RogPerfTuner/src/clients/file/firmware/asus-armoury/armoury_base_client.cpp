#include "clients/file/firmware/asus-armoury/armoury_base_client.hpp"

#include <stdexcept>
#include <string>

#include "utils/file_utils.hpp"
#include "utils/string_utils.hpp"

ArmouryBaseClient::ArmouryBaseClient(std::string attribute, bool required)
	: AbstractFileClient("/sys/class/firmware-attributes/asus-armoury/attributes/" + attribute + "/current_value", attribute, true, required),
	  attributePath("/sys/class/firmware-attributes/asus-armoury/attributes/" + attribute) {
}

int ArmouryBaseClient::getCurrentValue() {
	return stoi(read());
}

void ArmouryBaseClient::setCurrentValue(int value) {
	auto min = getMinValue();
	auto max = getMaxValue();

	if (value < min || value > max) {
		throw std::runtime_error("Value " + std::to_string(value) + " outside of range [" + std::to_string(min) + "," + std::to_string(max) + "]");
	}

	write(std::to_string(value));
}

int ArmouryBaseClient::getMaxValue() {
	if (FileUtils::exists(attributePath + "/max_value")) {
		return stoi(FileUtils::readFileContent(attributePath + "/max_value"));
	}
	if (FileUtils::exists(attributePath + "/possible_values")) {
		const auto splitted = StringUtils::split(FileUtils::readFileContent(attributePath + "/possible_values"), ';');
		return stoi(splitted[splitted.size() - 1]);
	}
	return 0;
}

int ArmouryBaseClient::getMinValue() {
	if (FileUtils::exists(attributePath + "/min_value")) {
		return stoi(FileUtils::readFileContent(attributePath + "/min_value"));
	}
	if (FileUtils::exists(attributePath + "/possible_values")) {
		return stoi(StringUtils::split(FileUtils::readFileContent(attributePath + "/possible_values"), ';')[0]);
	}
	return 0;
}

bool ArmouryBaseClient::available() {
	return AbstractFileClient::available();
}