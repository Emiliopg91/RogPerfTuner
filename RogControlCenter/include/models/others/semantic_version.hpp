#pragma once

#include <string>

struct SemanticVersion {
	int major;
	int minor;
	int patch;

	static SemanticVersion parse(const std::string& version_str);

	bool operator==(const SemanticVersion& other) const;

	bool operator!=(const SemanticVersion& other) const;

	bool operator<(const SemanticVersion& other) const;

	bool operator>(const SemanticVersion& other) const;

	bool operator<=(const SemanticVersion& other) const;

	bool operator>=(const SemanticVersion& other) const;
};
