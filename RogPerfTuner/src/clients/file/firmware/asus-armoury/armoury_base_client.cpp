#include "clients/file/firmware/asus-armoury/armoury_base_client.hpp"

#include <stdexcept>
#include <string>

#include "framework/utils/file_utils.hpp"
#include "framework/utils/string_utils.hpp"

// TODO: restore previous

ArmouryBaseClient::ArmouryBaseClient(std::string attribute, bool required)
	: AbstractFileClient("/sys/class/firmware-attributes/asus-armoury/attributes/" + attribute + "/current_value", attribute, true, required),
	  attributePath("/sys/class/firmware-attributes/asus-armoury/attributes/" + attribute) {
	available_ = false;

	if (AbstractFileClient::available()) {
		try {
			getMinValue();
			getMaxValue();
			getCurrentValue();
			available_ = true;
		} catch (std::exception& e) {
			if (required) {
				throw std::runtime_error(std::format("Firmware attribute {} not available due to {}", attribute, e.what()));
			}
		}
	}
}

int ArmouryBaseClient::getCurrentValue() {
	auto value_str = read();
	try {
		return std::stoi(StringUtils::trim(value_str));
	} catch (const std::exception& e) {
		logger->error("stoi failed for value '{}': {}", value_str, e.what());
		throw;
	}
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
	std::string content;
	try {
		if (FileUtils::exists(attributePath + "/max_value")) {
			content = FileUtils::readFileContent(attributePath + "/max_value");
			return stoi(StringUtils::trim(content));
		}
		if (FileUtils::exists(attributePath + "/possible_values")) {
			const auto splitted = StringUtils::split(FileUtils::readFileContent(attributePath + "/possible_values"), ';');
			content				= splitted[splitted.size() - 1];
			return stoi(StringUtils::trim(content));
		}
		return 0;
	} catch (const std::exception& e) {
		logger->error("stoi failed for max_value '{}': {}", content, e.what());
		throw;
	}
}

int ArmouryBaseClient::getMinValue() {
	std::string content;
	try {
		if (FileUtils::exists(attributePath + "/min_value")) {
			content = FileUtils::readFileContent(attributePath + "/min_value");
			return stoi(StringUtils::trim(content));
		}
		if (FileUtils::exists(attributePath + "/possible_values")) {
			const auto splitted = StringUtils::split(FileUtils::readFileContent(attributePath + "/possible_values"), ';');
			content				= splitted[0];
			return stoi(StringUtils::trim(content));
		}
		return 0;
	} catch (const std::exception& e) {
		logger->error("stoi failed for min_value '{}': {}", content, e.what());
		throw;
	}
}

bool ArmouryBaseClient::available() {
	return available_;
}