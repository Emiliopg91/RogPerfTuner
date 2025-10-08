#include "../../../include/models/others/semantic_version.hpp"

#include <algorithm>
#include <sstream>

SemanticVersion SemanticVersion::parse(const std::string& version_str) {
	std::string v = version_str;
	v.erase(v.begin(), std::find_if(v.begin(), v.end(), [](unsigned char ch) {
				return !std::isspace(ch);
			}));
	v.erase(std::find_if(v.rbegin(), v.rend(),
						 [](unsigned char ch) {
							 return !std::isspace(ch);
						 })
				.base(),
			v.end());

	std::istringstream ss(v);
	std::string token;
	SemanticVersion semver{0, 0, 0};

	if (std::getline(ss, token, '.')) {
		semver.major = std::stoi(token);
	}
	if (std::getline(ss, token, '.')) {
		semver.minor = std::stoi(token);
	}
	if (std::getline(ss, token, '.')) {
		semver.patch = std::stoi(token);
	}

	return semver;
}

bool SemanticVersion::operator==(const SemanticVersion& other) const {
	return major == other.major && minor == other.minor && patch == other.patch;
}

bool SemanticVersion::operator!=(const SemanticVersion& other) const {
	return !(*this == other);
}

bool SemanticVersion::operator<(const SemanticVersion& other) const {
	if (major != other.major) {
		return major < other.major;
	}
	if (minor != other.minor) {
		return minor < other.minor;
	}
	return patch < other.patch;
}

bool SemanticVersion::operator>(const SemanticVersion& other) const {
	return other < *this;
}

bool SemanticVersion::operator<=(const SemanticVersion& other) const {
	return !(other < *this);
}

bool SemanticVersion::operator>=(const SemanticVersion& other) const {
	return !(*this < other);
}