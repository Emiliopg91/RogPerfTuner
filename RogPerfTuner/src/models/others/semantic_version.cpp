#include "../../../include/models/others/semantic_version.hpp"

#include <algorithm>
#include <sstream>

SemanticVersion SemanticVersion::parse(const std::string& version_str) {
	std::string v = version_str;
	// Trim de espacios
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
	SemanticVersion semver{0, 0, 0, ""};

	if (std::getline(ss, token, '.')) {
		semver.major = std::stoi(token);
	}
	if (std::getline(ss, token, '.')) {
		semver.minor = std::stoi(token);
	}
	if (std::getline(ss, token, '.')) {
		// El patch puede venir acompañado de - o .
		size_t pos			  = token.find_first_of("-.");
		std::string patch_str = token.substr(0, pos);
		semver.patch		  = std::stoi(patch_str);

		if (pos != std::string::npos) {
			std::string remainder = token.substr(pos + 1);
			if (!remainder.empty()) {
				semver.sufix = remainder;
			}
		}
	}

	if (ss.peek() == '-' || ss.peek() == '.') {
		char sep;
		ss >> sep;
		std::getline(ss, semver.sufix);
	}

	return semver;
}

bool is_numeric(const std::string& s) {
	return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
		return std::isdigit(c);
	});
}

bool SemanticVersion::operator==(const SemanticVersion& other) const {
	if (major != other.major || minor != other.minor || patch != other.patch) {
		return false;
	}

	if (is_numeric(sufix) && is_numeric(other.sufix)) {
		return std::stoi(sufix) == std::stoi(other.sufix);
	}

	return true;
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
	if (patch != other.patch) {
		return patch < other.patch;
	}

	bool this_num  = is_numeric(sufix);
	bool other_num = is_numeric(other.sufix);

	if (this_num && other_num) {
		return std::stoi(sufix) < std::stoi(other.sufix);
	}

	if (sufix.empty() && !other.sufix.empty()) {
		return true;
	}
	if (!sufix.empty() && other.sufix.empty()) {
		return false;
	}

	return false;
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
